
#include <loader/ExeReader.hpp>

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

#include "loader/ExeFormat.hpp"
#include "util/Output.hpp"
#include "util/Utils.hpp"

using std::cout;
using std::string;
using std::endl;
using std::setw;
using std::setfill;
using std::hex;
using std::dec;

size_t ExeReader::findResourceEntry(std::istream & ifs, int id) {
	
	CoffResourceTable table;
	if(read(ifs, table).fail()) {
		LogError << "error reading resource table";
		return 0;
	}
	
	cout << "[table] char: " << table.characteristics << "  time: " << table.timestamp << "  version: " << table.majorVersion << '.' << table.minorVersion << "  names: " << table.nbnames << "  ids: " << table.nbids << endl;
	
	ifs.seekg(table.nbnames * sizeof(CoffResourceEntry), std::ios_base::cur);
	
	size_t offset = 0;
	
	for(size_t i = 0; i < table.nbids; i++) {
		
		CoffResourceEntry entry;
		if(read(ifs, entry).fail()) {
			LogError << "error reading resource table entry";
			return 0;
		}
		
		cout << "[entry] id: " << entry.id << "  address: " << hex << (entry.offset & ~(1 << 31)) << dec << "  type: " << ((entry.offset & (1 << 31)) ? "table" : "leaf" ) << endl;
		
		if(entry.id == id) {
			//ifs.seekg((table.nbids - i - 1) * sizeof(ResourceEntry));
			offset = entry.offset;
			// break;
		}
	}
	
	return offset;
}

bool ExeReader::loadSectionTable(std::istream & ifs, size_t peOffset, const CoffFileHeader & coff, CoffSectionTable & table) {
	
	size_t sectionTableOffset = peOffset + 4 + sizeof(CoffFileHeader) + coff.optionalHeaderSize;
	
	table.resize(coff.nsections);
	
	ifs.seekg(sectionTableOffset);
	if(ifs.read(reinterpret_cast<char *>(table.data()), sizeof(CoffSection) * table.size()).fail()) {
		LogError << "error coff loading section table";
		return false;
	}
	
	for(CoffSectionTable::const_iterator i = table.begin(); i != table.end(); ++i) {
		cout << "[section] \"" << safestring(i->name) << "\" virtual=" << hex << i->virtualAddress << '+' << i->virtualSize << " raw=" << i->rawAddress << '+' << i->rawSize << dec << endl;
	}
	
	return true;
}

size_t ExeReader::memoryAddressToFileOffset(const CoffSectionTable & sections, size_t memory) {
	
	for(CoffSectionTable::const_iterator i = sections.begin(); i != sections.end(); ++i) {
		
		const CoffSection & section = *i;
		
		if(memory >= section.virtualAddress && memory < section.virtualAddress + section.virtualSize) {
			return memory + section.rawAddress - section.virtualAddress;
		}
		
	}
	
	return 0;
}

ExeReader::Resource ExeReader::findResource(std::istream & is, int name, int type, int language) {
	
	Resource result;
	result.offset = result.size = 0;
	
	uint16_t peOffset;
	if(read(is.seekg(0x3c), peOffset).fail()) {
		LogError << "error reading PE signature offset";
		return result;
	}
	cout << "PE signature is @ " << hex << peOffset << dec << endl;
	
	char magic[4];
	if(is.seekg(peOffset).read(magic, 4).fail()) {
		LogError << "error reading PE signature";
		return result;
	}
	static const char expectedMagic[] = { 'P', 'E', 0, 0 };
	if(std::memcmp(magic, expectedMagic, 4)) {
		LogError << "wrong PE signature - not an exe file";
		return result;
	}
	
	CoffFileHeader coff;
	if(read(is, coff).fail()) {
		LogError << "error reading COFF file header";
		return result;
	}
	
	uint16_t optionalHeaderMagic;
	if(read(is, optionalHeaderMagic).fail()) {
		LogError << "error reading the optional header magic number";
		return result;
	}
	
	// skip the optional header
	if(optionalHeaderMagic == 0x20b) { // PE32+
		is.seekg(106, std::ios_base::cur);
	} else {
		is.seekg(90, std::ios_base::cur);
	}
	
	uint32_t ndirectories;
	if(read(is, ndirectories).fail()) {
		LogError << "error reading number of data directories";
		return result;
	}
	cout << "number of directories is " << ndirectories << endl;
	if(ndirectories < 3) {
		LogError << "no resource directory found";
		return result;
	}
	
	CoffDataDirectory resources;
	if(read(is.seekg(16, std::ios_base::cur), resources).fail()) {
		LogError << "error reading resource directory offset";
		return result;
	}
	if(!resources.address || !resources.size) {
		LogError << "missing resource directory";
		return result;
	}
	
	CoffSectionTable sections;
	if(!loadSectionTable(is, peOffset, coff, sections)) {
		return result;
	}
	
	size_t resourceOffset = memoryAddressToFileOffset(sections, resources.address);
	if(!resourceOffset) {
		LogError << "error mapping virtual resource address " << hex << resources.address << dec << " to file offset";
		return result;
	}
	cout << "resource table is @ RVA " << hex << resources.address << " -> @ " << resourceOffset << dec << endl;
	
	is.seekg(resourceOffset);
	
	uint32_t typeOffset = findResourceEntry(is, type);
	if(!typeOffset) {
		LogError << "missing data resource entry";
		return result;
	}
	if(!(typeOffset & (1 << 31))) {
		LogError << "unexpected resource leaf for data";
		return result;
	}
	typeOffset &= ~(1 << 31), typeOffset += resourceOffset;
	
	cout << "data resource table is @ " << hex << typeOffset << dec << endl;
	
	is.seekg(typeOffset);
	
	size_t nameOffset = findResourceEntry(is, name);
	if(!nameOffset) {
		LogError << "missing installer resource entry";
		return result;
	}
	if(!(nameOffset & (1 << 31))) {
		LogError << "unexpected resource leaf for installer";
		return result;
	}
	nameOffset &= ~(1 << 31), nameOffset += resourceOffset;
	
	cout << "installer resource table is @ " << hex << nameOffset << dec << endl;
	
	is.seekg(nameOffset);
	
	size_t finalOffset = findResourceEntry(is, language);
	if(!finalOffset) {
		LogError << "missing final resource entry";
		return result;
	}
	if(finalOffset & (1 << 31)) {
		LogError << "unexpected table for final resource entry";
		return result;
	}
	finalOffset += resourceOffset;
	
	cout << "final resource entry is @ " << hex << finalOffset << dec << endl;
	
	CoffResourceLeaf leaf;
	if(read(is.seekg(finalOffset), leaf).fail()) {
		LogError << "error loading final resource entry";
		return result;
	}
	
	cout << "[resource] address: " << hex << leaf.address << dec << "  size: " << leaf.size << "  codepage: " << leaf.codepage << endl;
	
	size_t dataOffset = memoryAddressToFileOffset(sections, leaf.address);
	if(!dataOffset) {
		LogError << "error mapping final virtual resource address " << hex << leaf.address << dec << " to file offset";
		return result;
	}
	
	result.offset = dataOffset;
	result.size = leaf.size;
	
	return result;
}

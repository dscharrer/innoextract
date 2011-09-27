
#include "ExeReader.hpp"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

#include "Types.hpp"
#include "ExeFormat.hpp"
#include "Utils.hpp"
#include "Output.hpp"

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
		error << "error reading resource table";
		return 0;
	}
	
	cout << "[table] char: " << table.characteristics << "  time: " << table.timestamp << "  version: " << table.majorVersion << '.' << table.minorVersion << "  names: " << table.nbnames << "  ids: " << table.nbids << endl;
	
	ifs.seekg(table.nbnames * sizeof(CoffResourceEntry), strm::cur);
	
	size_t offset = 0;
	
	for(size_t i = 0; i < table.nbids; i++) {
		
		CoffResourceEntry entry;
		if(read(ifs, entry).fail()) {
			error << "error reading resource table entry";
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
		error << "error coff loading section table";
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
	
	u16 peOffset;
	if(read(is.seekg(0x3c), peOffset).fail()) {
		error << "error reading PE signature offset";
		return result;
	}
	cout << "PE signature is @ " << hex << peOffset << dec << endl;
	
	char magic[4];
	if(is.seekg(peOffset).read(magic, 4).fail()) {
		error << "error reading PE signature";
		return result;
	}
	static const char expectedMagic[] = { 'P', 'E', 0, 0 };
	if(std::memcmp(magic, expectedMagic, 4)) {
		error << "wrong PE signature - not an exe file";
		return result;
	}
	
	CoffFileHeader coff;
	if(read(is, coff).fail()) {
		error << "error reading COFF file header";
		return result;
	}
	
	u16 optionalHeaderMagic;
	if(read(is, optionalHeaderMagic).fail()) {
		error << "error reading the optional header magic number";
		return result;
	}
	
	// skip the optional header
	if(optionalHeaderMagic == 0x20b) { // PE32+
		is.seekg(106, strm::cur);
	} else {
		is.seekg(90, strm::cur);
	}
	
	u32 ndirectories;
	if(read(is, ndirectories).fail()) {
		error << "error reading number of data directories";
		return result;
	}
	cout << "number of directories is " << ndirectories << endl;
	if(ndirectories < 3) {
		error << "no resource directory found";
		return result;
	}
	
	CoffDataDirectory resources;
	if(read(is.seekg(16, strm::cur), resources).fail()) {
		error << "error reading resource directory offset";
		return result;
	}
	if(!resources.address || !resources.size) {
		error << "missing resource directory";
		return result;
	}
	
	CoffSectionTable sections;
	if(!loadSectionTable(is, peOffset, coff, sections)) {
		return result;
	}
	
	size_t resourceOffset = memoryAddressToFileOffset(sections, resources.address);
	if(!resourceOffset) {
		error << "error mapping virtual resource address " << hex << resources.address << dec << " to file offset";
		return result;
	}
	cout << "resource table is @ RVA " << hex << resources.address << " -> @ " << resourceOffset << dec << endl;
	
	is.seekg(resourceOffset);
	
	u32 typeOffset = findResourceEntry(is, type);
	if(!typeOffset) {
		error << "missing data resource entry";
		return result;
	}
	if(!(typeOffset & (1 << 31))) {
		error << "unexpected resource leaf for data";
		return result;
	}
	typeOffset &= ~(1 << 31), typeOffset += resourceOffset;
	
	cout << "data resource table is @ " << hex << typeOffset << dec << endl;
	
	is.seekg(typeOffset);
	
	u32 nameOffset = findResourceEntry(is, name);
	if(!nameOffset) {
		error << "missing installer resource entry";
		return result;
	}
	if(!(nameOffset & (1 << 31))) {
		error << "unexpected resource leaf for installer";
		return result;
	}
	nameOffset &= ~(1 << 31), nameOffset += resourceOffset;
	
	cout << "installer resource table is @ " << hex << nameOffset << dec << endl;
	
	is.seekg(nameOffset);
	
	u32 finalOffset = findResourceEntry(is, language);
	if(!finalOffset) {
		error << "missing final resource entry";
		return result;
	}
	if(finalOffset & (1 << 31)) {
		error << "unexpected table for final resource entry";
		return result;
	}
	finalOffset += resourceOffset;
	
	cout << "final resource entry is @ " << hex << finalOffset << dec << endl;
	
	CoffResourceLeaf leaf;
	if(read(is.seekg(finalOffset), leaf).fail()) {
		error << "error loading final resource entry";
		return result;
	}
	
	cout << "[resource] address: " << hex << leaf.address << dec << "  size: " << leaf.size << "  codepage: " << leaf.codepage << endl;
	
	size_t dataOffset = memoryAddressToFileOffset(sections, leaf.address);
	if(!dataOffset) {
		error << "error mapping final virtual resource address " << hex << leaf.address << dec << " to file offset";
		return result;
	}
	
	result.offset = dataOffset;
	result.size = leaf.size;
	
	return result;
}

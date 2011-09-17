
#include "ExeReader.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

#include "ExeFormat.h"
#include "Types.h"
#include "Utils.h"
#include "../shady/transform.hpp"

using std::cout;
using std::cerr;
using std::string;
using std::endl;
using std::setw;
using std::setfill;
using std::hex;
using std::dec;

size_t ExeReader::findResourceEntry(std::istream & ifs, int id) {
	
	ResourceTable table;
	if(read(ifs, table).fail()) {
		cerr << "error reading resource table" << endl;
		return 0;
	}
	
	cout << "[table] char: " << table.characteristics << "  time: " << table.timestamp << "  version: " << table.majorVersion << '.' << table.minorVersion << "  names: " << table.nbnames << "  ids: " << table.nbids << endl;
	
	ifs.seekg(table.nbnames * sizeof(ResourceEntry), strm::cur);
	
	size_t offset = 0;
	
	for(size_t i = 0; i < table.nbids; i++) {
		
		ResourceEntry entry;
		if(read(ifs, entry).fail()) {
			cerr << "error reading resource table entry" << endl;
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
		cerr << "error coff loading section table";
		return false;
	}
	
	for(CoffSectionTable::const_iterator i = table.begin(); i != table.end(); ++i) {
		
		const CoffSection & section = *i;
		
		cout << "[section] \"" << safestring(section.name) << "\" virtual=" << hex << section.virtualAddress << '+' << section.virtualSize << " raw=" << section.rawAddress << '+' << section.rawSize << dec << endl;
		
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
		cerr << "error reading PE signature offset";
		return result;
	}
	cout << "PE signature is @ " << hex << peOffset << dec << endl;
	
	char magic[4];
	if(is.seekg(peOffset).read(magic, 4).fail()) {
		cerr << "error reading PE signature" << endl;
		return result;
	}
	static const char expectedMagic[] = { 'P', 'E', 0, 0 };
	if(std::memcmp(magic, expectedMagic, 4)) {
		cerr << "wrong PE signature - not an exe file" << endl;
		return result;
	}
	
	CoffFileHeader coff;
	if(read(is, coff).fail()) {
		cerr << "error reading COFF file header" << endl;
		return result;
	}
	
	u16 optionalHeaderMagic;
	if(read(is, optionalHeaderMagic).fail()) {
		cerr << "error reading the optional header magic number" << endl;
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
		cerr << "error reading number of data directories" << endl;
		return result;
	}
	cout << "number of directories is " << ndirectories << endl;
	if(ndirectories < 3) {
		cerr << "no resource directory found" << endl;
		return result;
	}
	
	DataDirectory resources;
	if(read(is.seekg(16, strm::cur), resources).fail()) {
		cerr << "error reading resource directory offset";
		return result;
	}
	if(!resources.address || !resources.size) {
		cerr << "missing resource directory" << endl;
		return result;
	}
	
	CoffSectionTable sections;
	if(!loadSectionTable(is, peOffset, coff, sections)) {
		return result;
	}
	
	size_t resourceOffset = memoryAddressToFileOffset(sections, resources.address);
	if(!resourceOffset) {
		cerr << "error mapping virtual resource address " << hex << resources.address << dec << " to file offset" << endl;
		return result;
	}
	cout << "resource table is @ RVA " << hex << resources.address << " -> @ " << resourceOffset << dec << endl;
	
	is.seekg(resourceOffset);
	
	u32 typeOffset = findResourceEntry(is, type);
	if(!typeOffset) {
		cerr << "missing data resource entry" << endl;
		return result;
	}
	if(!(typeOffset & (1 << 31))) {
		cerr << "unexpected resource leaf for data" << endl;
		return result;
	}
	typeOffset &= ~(1 << 31), typeOffset += resourceOffset;
	
	cout << "data resource table is @ " << hex << typeOffset << dec << endl;
	
	is.seekg(typeOffset);
	
	u32 nameOffset = findResourceEntry(is, name);
	if(!nameOffset) {
		cerr << "missing installer resource entry" << endl;
		return result;
	}
	if(!(nameOffset & (1 << 31))) {
		cerr << "unexpected resource leaf for installer" << endl;
		return result;
	}
	nameOffset &= ~(1 << 31), nameOffset += resourceOffset;
	
	cout << "installer resource table is @ " << hex << nameOffset << dec << endl;
	
	is.seekg(nameOffset);
	
	u32 finalOffset = findResourceEntry(is, language);
	if(!finalOffset) {
		cerr << "missing final resource entry" << endl;
		return result;
	}
	if(finalOffset & (1 << 31)) {
		cerr << "unexpected table for final resource entry" << endl;
		return result;
	}
	finalOffset += resourceOffset;
	
	cout << "final resource entry is @ " << hex << finalOffset << dec << endl;
	
	ResourceLeaf leaf;
	if(read(is.seekg(finalOffset), leaf).fail()) {
		cerr << "error loading final resource entry" << endl;
		return result;
	}
	
	cout << "[resource] address: " << hex << leaf.address << dec << "  size: " << leaf.size << "  codepage: " << leaf.codepage << endl;
	
	size_t dataOffset = memoryAddressToFileOffset(sections, leaf.address);
	if(!dataOffset) {
		cerr << "error mapping final virtual resource address " << hex << leaf.address << dec << " to file offset" << endl;
		return result;
	}
	
	result.offset = dataOffset;
	result.size = leaf.size;
	
	return result;
}

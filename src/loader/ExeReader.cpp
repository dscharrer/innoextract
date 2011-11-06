
#include <loader/ExeReader.hpp>

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

#include "util/LoadingUtils.hpp"
#include "util/Utils.hpp"

namespace {

static const char PE_MAGIC[] = { 'P', 'E', 0, 0 };

inline bool getResourceTable(uint32_t & entry, uint32_t resource_offset) {
	
	bool is_table = (entry & (uint32_t(1) << 31));
	
	entry &= ~(1 << 31), entry += resource_offset;
	
	return is_table;
}

} // anonymous namespace

struct ExeReader::CoffFileHeader {
	
	//! Number of CoffSection structures following this header after optionalHeaderSize bytes.
	uint16_t nsections;
	
	//! Offset from the end of this header to the start of the section table.
	uint16_t optional_header_size;
	
};

struct ExeReader::CoffSection {
	
	uint32_t virtual_size; //!< Section size in virtual memory.
	uint32_t virtual_address; //!< Base virtual memory address.
	
	uint32_t raw_address; //!< Base file offset.
	
};

uint32_t ExeReader::findResourceEntry(std::istream & is, uint32_t needle) {
	
	// skip: characteristics + timestamp + major version + minor version
	if(is.seekg(4 + 4 + 2 + 2, std::ios_base::cur).fail()) {
		return 0;
	}
	
	// Number of named resource entries.
	uint16_t nbnames = loadNumber<uint16_t>(is);
	
	// Number of id resource entries.
	uint16_t nbids = loadNumber<uint16_t>(is);
	
	
	// Ignore named resource entries.
	const uint32_t entry_size = 4 + 4; // id / string address + offset
	if(is.seekg(nbnames * entry_size, std::ios_base::cur).fail()) {
		return 0;
	}
	
	for(size_t i = 0; i < nbids; i++) {
		
		uint32_t id = loadNumber<uint32_t>(is);
		uint32_t offset = loadNumber<uint32_t>(is);
		if(is.fail()) {
			return 0;
		}
		
		if(id == needle) {
			return offset;
		}
	}
	
	return 0;
}

bool ExeReader::loadSectionTable(std::istream & is, uint32_t peOffset,
                                 const CoffFileHeader & coff, CoffSectionTable & table) {
	
	// machine + nsections + creation time + symbol table offset + nsymbols
	// + optional header size + characteristics
	const uint32_t file_header_size = 2 + 2 + 4 + 4 + 4 + 2 + 2;
	uint32_t section_table_offset = peOffset + uint32_t(sizeof(PE_MAGIC)) + file_header_size
	                              + coff.optional_header_size;
	is.seekg(section_table_offset);
	
	table.resize(coff.nsections);
	for(CoffSectionTable::iterator i = table.begin(); i != table.end(); ++i) {
		CoffSection & section = *i;
		
		is.seekg(8, std::ios_base::cur); // name
		
		section.virtual_size = loadNumber<uint32_t>(is);
		section.virtual_address = loadNumber<uint32_t>(is);
		
		is.seekg(4, std::ios_base::cur); // raw size
		section.raw_address = loadNumber<uint32_t>(is);
		
		// relocation addr + line number addr + relocation count + line number count + characteristics
		is.seekg(4 + 4 + 2 + 2 + 4, std::ios_base::cur);
		
	}
	
	return !is.fail();
}

uint32_t ExeReader::memoryAddressToFileOffset(const CoffSectionTable & sections, uint32_t memory) {
	
	for(CoffSectionTable::const_iterator i = sections.begin(); i != sections.end(); ++i) {
		const CoffSection & section = *i;
		
		if(memory >= section.virtual_address
			 && memory < section.virtual_address + section.virtual_size) {
			return memory + section.raw_address - section.virtual_address;
		}
		
	}
	
	return 0;
}

ExeReader::Resource ExeReader::findResource(std::istream & is, uint32_t name,
                                            uint32_t type, uint32_t language) {
	
	Resource result;
	result.offset = result.size = 0;
	
	// Skip the DOS stub.
	uint16_t peOffset = loadNumber<uint16_t>(is.seekg(0x3c));
	if(is.fail()) {
		return result;
	}
	
	char magic[sizeof(PE_MAGIC)];
	if(is.seekg(peOffset).read(magic, sizeof(magic)).fail()) {
		return result;
	}
	if(std::memcmp(magic, PE_MAGIC, sizeof(PE_MAGIC))) {
		return result;
	}
	
	CoffFileHeader coff;
	is.seekg(2, std::ios_base::cur); // machine
	coff.nsections = loadNumber<uint16_t>(is);
	is.seekg(4 + 4 + 4, std::ios_base::cur); // creation time + symbol table offset + nbsymbols
	coff.optional_header_size = loadNumber<uint16_t>(is);
	is.seekg(2, std::ios_base::cur); // characteristics
	
	// Skip the optional header.
	uint16_t optionalHeaderMagic = loadNumber<uint16_t>(is);
	if(is.fail()) {
		return result;
	}
	if(optionalHeaderMagic == 0x20b) { // PE32+
		is.seekg(106, std::ios_base::cur);
	} else {
		is.seekg(90, std::ios_base::cur);
	}
	
	uint32_t ndirectories = loadNumber<uint32_t>(is);
	if(is.fail() || ndirectories < 3) {
		return result;
	}
	const uint32_t directory_header_size = 4 + 4; // address + size
	is.seekg(2 * directory_header_size, std::ios_base::cur);
	
	// Virtual memory address and size of the start of resource directory.
	uint32_t resource_address = loadNumber<uint32_t>(is);
	uint32_t resource_size = loadNumber<uint32_t>(is);
	if(is.fail() || !resource_address || !resource_size) {
		return result;
	}
	
	CoffSectionTable sections;
	if(!loadSectionTable(is, peOffset, coff, sections)) {
		return result;
	}
	
	uint32_t resource_offset = memoryAddressToFileOffset(sections, resource_address);
	if(!resource_offset) {
		return result;
	}
	
	is.seekg(resource_offset);
	uint32_t type_offset = findResourceEntry(is, type);
	if(!getResourceTable(type_offset, resource_offset)) {
		return result;
	}
	
	is.seekg(type_offset);
	uint32_t name_offset = findResourceEntry(is, name);
	if(!getResourceTable(name_offset, resource_offset)) {
		return result;
	}
	
	is.seekg(name_offset);
	uint32_t leaf_offset = findResourceEntry(is, language);
	if(!leaf_offset || getResourceTable(leaf_offset, resource_offset)) {
		return result;
	}
	
	// Virtual memory address and size of the resource data.
	is.seekg(leaf_offset);
	uint32_t data_address = loadNumber<uint32_t>(is);
	uint32_t data_size = loadNumber<uint32_t>(is);
	// ignore codepage and reserved word
	if(is.fail()) {
		return result;
	}
	
	uint32_t data_offset = memoryAddressToFileOffset(sections, data_address);
	if(!data_offset) {
		return result;
	}
	
	result.offset = data_offset;
	result.size = data_size;
	
	return result;
}

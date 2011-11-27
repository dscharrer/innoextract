
#include "loader/exereader.hpp"

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

#include "util/load.hpp"

namespace loader {

namespace {

static const char PE_MAGIC[] = { 'P', 'E', 0, 0 };

inline bool get_resource_table(uint32_t & entry, uint32_t resource_offset) {
	
	bool is_table = (entry & (uint32_t(1) << 31));
	
	entry &= ~(1 << 31), entry += resource_offset;
	
	return is_table;
}

} // anonymous namespace

struct exe_reader::header {
	
	//! Number of CoffSection structures following this header after optionalHeaderSize bytes.
	uint16_t nsections;
	
	//! Offset of the section table in the file.
	uint32_t section_table_offset;
	
	//! Virtual memory address of the resource root table.
	uint32_t resource_table_address;
	
};

struct exe_reader::section {
	
	uint32_t virtual_size; //!< Section size in virtual memory.
	uint32_t virtual_address; //!< Base virtual memory address.
	
	uint32_t raw_address; //!< Base file offset.
	
};

uint32_t exe_reader::find_resource_entry(std::istream & is, uint32_t needle) {
	
	// skip: characteristics + timestamp + major version + minor version
	if(is.seekg(4 + 4 + 2 + 2, std::ios_base::cur).fail()) {
		return 0;
	}
	
	// Number of named resource entries.
	uint16_t nbnames = load_number<uint16_t>(is);
	
	// Number of id resource entries.
	uint16_t nbids = load_number<uint16_t>(is);
	
	
	// Ignore named resource entries.
	const uint32_t entry_size = 4 + 4; // id / string address + offset
	if(is.seekg(nbnames * entry_size, std::ios_base::cur).fail()) {
		return 0;
	}
	
	for(size_t i = 0; i < nbids; i++) {
		
		uint32_t id = load_number<uint32_t>(is);
		uint32_t offset = load_number<uint32_t>(is);
		if(is.fail()) {
			return 0;
		}
		
		if(id == needle) {
			return offset;
		}
	}
	
	return 0;
}

bool exe_reader::load_header(std::istream & is, header & coff) {
	
	// Skip the DOS stub.
	uint16_t peOffset = load_number<uint16_t>(is.seekg(0x3c));
	if(is.fail()) {
		return false;
	}
	
	char magic[sizeof(PE_MAGIC)];
	if(is.seekg(peOffset).read(magic, sizeof(magic)).fail()) {
		return false;
	}
	if(std::memcmp(magic, PE_MAGIC, sizeof(PE_MAGIC))) {
		return false;
	}
	
	is.seekg(2, std::ios_base::cur); // machine
	coff.nsections = load_number<uint16_t>(is);
	is.seekg(4 + 4 + 4, std::ios_base::cur); // creation time + symbol table offset + nbsymbols
	uint16_t optional_header_size = load_number<uint16_t>(is);
	is.seekg(2, std::ios_base::cur); // characteristics
	
	coff.section_table_offset = uint32_t(is.tellg()) + optional_header_size;
	
	// Skip the optional header.
	uint16_t optionalHeaderMagic = load_number<uint16_t>(is);
	if(is.fail()) {
		return false;
	}
	if(optionalHeaderMagic == 0x20b) { // PE32+
		is.seekg(106, std::ios_base::cur);
	} else {
		is.seekg(90, std::ios_base::cur);
	}
	
	uint32_t ndirectories = load_number<uint32_t>(is);
	if(is.fail() || ndirectories < 3) {
		return false;
	}
	const uint32_t directory_header_size = 4 + 4; // address + size
	is.seekg(2 * directory_header_size, std::ios_base::cur);
	
	// Virtual memory address and size of the start of resource directory.
	coff.resource_table_address = load_number<uint32_t>(is);
	uint32_t resource_size = load_number<uint32_t>(is);
	if(is.fail() || !coff.resource_table_address || !resource_size) {
		return false;
	}
	
	return true;
}

bool exe_reader::load_section_list(std::istream & is, const header & coff, section_list & table) {
	
	is.seekg(coff.section_table_offset);
	
	table.resize(coff.nsections);
	for(section_list::iterator i = table.begin(); i != table.end(); ++i) {
		section & section = *i;
		
		is.seekg(8, std::ios_base::cur); // name
		
		section.virtual_size = load_number<uint32_t>(is);
		section.virtual_address = load_number<uint32_t>(is);
		
		is.seekg(4, std::ios_base::cur); // raw size
		section.raw_address = load_number<uint32_t>(is);
		
		// relocation addr + line number addr + relocation count + line number count + characteristics
		is.seekg(4 + 4 + 2 + 2 + 4, std::ios_base::cur);
		
	}
	
	return !is.fail();
}

uint32_t exe_reader::to_file_offset(const section_list & sections, uint32_t memory) {
	
	for(section_list::const_iterator i = sections.begin(); i != sections.end(); ++i) {
		const section & s = *i;
		if(memory >= s.virtual_address && memory < s.virtual_address + s.virtual_size) {
			return memory + s.raw_address - s.virtual_address;
		}
	}
	
	return 0;
}

exe_reader::resource exe_reader::find_resource(std::istream & is, uint32_t name,
                                               uint32_t type, uint32_t language) {
	
	resource result;
	result.offset = result.size = 0;
	
	header coff;
	if(!load_header(is, coff)) {
		return result;
	}
	
	section_list sections;
	if(!load_section_list(is, coff, sections)) {
		return result;
	}
	
	uint32_t resource_offset = to_file_offset(sections, coff.resource_table_address);
	if(!resource_offset) {
		return result;
	}
	
	is.seekg(resource_offset);
	uint32_t type_offset = find_resource_entry(is, type);
	if(!get_resource_table(type_offset, resource_offset)) {
		return result;
	}
	
	is.seekg(type_offset);
	uint32_t name_offset = find_resource_entry(is, name);
	if(!get_resource_table(name_offset, resource_offset)) {
		return result;
	}
	
	is.seekg(name_offset);
	uint32_t leaf_offset = find_resource_entry(is, language);
	if(!leaf_offset || get_resource_table(leaf_offset, resource_offset)) {
		return result;
	}
	
	// Virtual memory address and size of the resource data.
	is.seekg(leaf_offset);
	uint32_t data_address = load_number<uint32_t>(is);
	uint32_t data_size = load_number<uint32_t>(is);
	// ignore codepage and reserved word
	if(is.fail()) {
		return result;
	}
	
	uint32_t data_offset = to_file_offset(sections, data_address);
	if(!data_offset) {
		return result;
	}
	
	result.offset = data_offset;
	result.size = data_size;
	
	return result;
}

} // namespace loader

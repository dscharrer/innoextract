/*
 * Copyright (C) 2011-2014 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "loader/exereader.hpp"

#include <iomanip>
#include <ios>
#include <istream>
#include <algorithm>
#include <cstring>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>

#include "util/load.hpp"

namespace loader {

namespace {

enum BinaryType {
	UnknownBinary = 0,
	DOSMagic = 0x5a4d, // "MZ"
	OS2Magic = 0x454E, // "NE"
	VXDMagic = 0x454C, // "LE"
	PEMagic  = 0x4550, // "PE"
	PEMagic2 = 0x0000, // "\0\0"
};

BinaryType determine_binary_type(std::istream & is) {
	
	boost::uint16_t dos_magic = util::load<boost::uint16_t>(is.seekg(0));
	if(is.fail() || dos_magic != DOSMagic) {
		return UnknownBinary; // Not a DOS file
	}
	
	// Skip the DOS stub
	boost::uint16_t new_offset = util::load<boost::uint16_t>(is.seekg(0x3c));
	if(is.fail()) {
		return DOSMagic;
	}
	
	boost::uint16_t new_magic = util::load<boost::uint16_t>(is.seekg(new_offset));
	if(is.fail()) {
		return DOSMagic;
	}
	
	if(new_magic == PEMagic) {
		boost::uint16_t pe2_magic = util::load<boost::uint16_t>(is);
		if(is.fail() || pe2_magic != PEMagic2) {
			return DOSMagic;
		}
	}
	
	return BinaryType(new_magic);
}

template <typename T>
bool skip_to_fixed_file_info(std::istream & is, boost::uint32_t resource_offset,
                             boost::uint32_t offset) {
	
	is.seekg(resource_offset + offset);
	T key;
	do {
		key = util::load<T>(is);
		if(is.fail()) {
			return false;
		}
		offset += boost::uint32_t(sizeof(key));
	} while(key != 0);
	
	// Align to DWORD
	offset = (offset + 3) & ~boost::uint32_t(3);
	
	is.seekg(resource_offset + offset);
	
	return true;
}

// Reader for OS2 binaries
struct ne_reader : public exe_reader {
	
	static resource find_resource(std::istream & is, boost::uint32_t name,
	                              boost::uint32_t type = TypeData);
	
	static bool get_file_version(std::istream & is);
	
};

exe_reader::resource ne_reader::find_resource(std::istream & is, boost::uint32_t name,
                                              boost::uint32_t type) {
	
	resource result;
	result.offset = result.size = 0;
	
	is.seekg(0x24 - 2, std::ios_base::cur); // Already read the magic
	boost::uint16_t resources_offset = util::load<boost::uint16_t>(is);
	boost::uint16_t resources_end = util::load<boost::uint16_t>(is);
	if(is.fail()) {
		return result;
	}
	
	if(resources_end == resources_offset) {
		return result;
	}
	
	is.seekg(std::streamoff(resources_offset) - 0x28, std::ios_base::cur);
	
	boost::uint16_t shift = util::load<boost::uint16_t>(is);
	if(is.fail() || shift >= 32) {
		return result;
	}
	
	boost::uint16_t name_count;
	for(;;) {
		
		boost::uint16_t type_id = util::load<boost::uint16_t>(is);
		name_count = util::load<boost::uint16_t>(is);
		is.seekg(4, std::ios_base::cur);
		if(is.fail() || type_id == 0) {
			return result;
		}
		
		if(type_id == boost::uint16_t(type | 0x8000)) {
			break;
		}
		
		is.seekg(name_count * 12, std::ios_base::cur);
		
	}
	
	for(boost::uint16_t i = 0; i < name_count; i++) {
		
		boost::uint16_t offset = util::load<boost::uint16_t>(is);
		boost::uint16_t size   = util::load<boost::uint16_t>(is);
		is.seekg(2, std::ios_base::cur);
		boost::uint16_t name_id = util::load<boost::uint16_t>(is);
		is.seekg(4, std::ios_base::cur);
		if(is.fail()) {
			return result;
		}
		
		if(name_id == boost::uint16_t(name | 0x8000)) {
			result.offset = boost::uint32_t(offset) << shift;
			result.size   = boost::uint32_t(size)   << shift;
			break;
		}
		
	}
	
	return result;
}

bool ne_reader::get_file_version(std::istream & is) {
	
	resource res = find_resource(is, NameVersionInfo, TypeVersion);
	if(!res) {
		return false;
	}
	
	return skip_to_fixed_file_info<boost::int8_t>(is, res.offset, 4);
}

// Reader for VXD binaries
struct le_reader : public exe_reader {
	
	static bool get_file_version(std::istream & is);
	
};

bool le_reader::get_file_version(std::istream & is) {
	
	is.seekg(0xb8 - 2, std::ios_base::cur);  // Already read the magic
	boost::uint32_t resources_offset = util::load<boost::uint32_t>(is);
	boost::uint32_t resources_size = util::load<boost::uint32_t>(is);
	if(is.fail()) {
		return false;
	}
	
	if(resources_size <= 12) {
		return false;
	}
	
	is.seekg(resources_offset);
	boost::uint8_t type = util::load<boost::uint8_t>(is);
	boost::uint16_t id = util::load<boost::uint16_t>(is);
	boost::uint8_t name = util::load<boost::uint8_t>(is);
	is.seekg(4, std::ios_base::cur); // skip ordinal + flags
	boost::uint32_t size = util::load<boost::uint32_t>(is);
	if(is.fail() || type != 0xff || id != 16 || name != 0xff || size <= 20 + 52) {
		return false;
	}
	
	boost::uint16_t node = util::load<boost::uint16_t>(is);
	boost::uint16_t data = util::load<boost::uint16_t>(is);
	is.seekg(16, std::ios_base::cur); // skip key
	if(is.fail() || node < 20 + 52 || data < 52) {
		return false;
	}
	
	return true;
}

// Reader for Win32 binaries
struct pe_reader : public exe_reader {
	
	struct header {
		
		//! Number of CoffSection structures following this header after optionalHeaderSize bytes
		boost::uint16_t nsections;
		
		//! Offset of the section table in the file
		boost::uint32_t section_table_offset;
		
		//! Virtual memory address of the resource root table
		boost::uint32_t resource_table_address;
		
		bool load(std::istream & is);
		
	};
	
	struct section {
		
		boost::uint32_t virtual_size; //!< Section size in virtual memory
		boost::uint32_t virtual_address; //!< Base virtual memory address
		
		boost::uint32_t raw_address; //!< Base file offset
		
	};
	
	struct section_table {
		
		std::vector<section> sections;
		
		bool load(std::istream & is, const header & coff);
		
		/*!
		 * Convert a memory address to a file offset according to the given section list.
		 */
		boost::uint32_t to_file_offset(boost::uint32_t address);
		
	};
	
	static bool get_resource_table(boost::uint32_t & entry, boost::uint32_t offset);
	
	/*!
	 * Find the entry in a resource table with a given ID.
	 * The input stream is expected to be positioned at the start of the table.
	 * The position if the stream after the function call is undefined.
	 *
	 * \return:
	 *   Highest order bit: 1 = points to another resource table
	 *                      0 = points to a resource leaf
	 *   Remaining 31 bits: Offset to the resource table / leaf relative to
	 *                      the directory start.
	 */
	static boost::uint32_t find_resource_entry(std::istream & is, boost::uint32_t id);
	
	static resource find_resource(std::istream & is, boost::uint32_t name,
	                              boost::uint32_t type = TypeData,
	                              boost::uint32_t language = Default);
	
	static bool get_file_version(std::istream & is);
	
};

bool pe_reader::header::load(std::istream & is) {
	
	is.seekg(2, std::ios_base::cur); // machine
	nsections = util::load<boost::uint16_t>(is);
	is.seekg(4 + 4 + 4, std::ios_base::cur); // creation time + symbol table offset + nbsymbols
	boost::uint16_t optional_header_size = util::load<boost::uint16_t>(is);
	is.seekg(2, std::ios_base::cur); // characteristics
	
	section_table_offset = boost::uint32_t(is.tellg()) + optional_header_size;
	
	// Skip the optional header.
	boost::uint16_t optional_header_magic = util::load<boost::uint16_t>(is);
	if(is.fail()) {
		return false;
	}
	if(optional_header_magic == 0x20b) { // PE32+
		is.seekg(106, std::ios_base::cur);
	} else {
		is.seekg(90, std::ios_base::cur);
	}
	
	boost::uint32_t ndirectories = util::load<boost::uint32_t>(is);
	if(is.fail() || ndirectories < 3) {
		return false;
	}
	const boost::uint32_t directory_header_size = 4 + 4; // address + size
	is.seekg(2 * directory_header_size, std::ios_base::cur);
	
	// Virtual memory address and size of the start of resource directory.
	resource_table_address = util::load<boost::uint32_t>(is);
	boost::uint32_t resource_size = util::load<boost::uint32_t>(is);
	if(is.fail() || !resource_table_address || !resource_size) {
		return false;
	}
	
	return true;
}

bool pe_reader::section_table::load(std::istream & is, const header & coff) {
	
	is.seekg(coff.section_table_offset);
	
	sections.resize(coff.nsections);
	
	BOOST_FOREACH(section & s, sections) {
		
		is.seekg(8, std::ios_base::cur); // name
		
		s.virtual_size = util::load<boost::uint32_t>(is);
		s.virtual_address = util::load<boost::uint32_t>(is);
		
		is.seekg(4, std::ios_base::cur); // raw size
		s.raw_address = util::load<boost::uint32_t>(is);
		
		// relocation addr + line number addr + relocation count
		// + line number count + characteristics
		is.seekg(4 + 4 + 2 + 2 + 4, std::ios_base::cur);
		
	}
	
	return !is.fail();
}

boost::uint32_t pe_reader::section_table::to_file_offset(boost::uint32_t address) {
	
	BOOST_FOREACH(const section & s, sections) {
		if(address >= s.virtual_address && address < s.virtual_address + s.virtual_size) {
			return address + s.raw_address - s.virtual_address;
		}
	}
	
	return 0;
}

bool pe_reader::get_resource_table(boost::uint32_t & entry, boost::uint32_t offset) {
	
	bool is_table = ((entry & (boost::uint32_t(1) << 31)) != 0);
	
	entry &= ~(1 << 31), entry += offset;
	
	return is_table;
}

boost::uint32_t pe_reader::find_resource_entry(std::istream & is, boost::uint32_t id) {
	
	// skip: characteristics + timestamp + major version + minor version
	if(is.seekg(4 + 4 + 2 + 2, std::ios_base::cur).fail()) {
		return 0;
	}
	
	// Number of named resource entries.
	boost::uint16_t nbnames = util::load<boost::uint16_t>(is);
	
	// Number of id resource entries.
	boost::uint16_t nbids = util::load<boost::uint16_t>(is);
	
	if(id == Default) {
		boost::uint32_t offset = util::load<boost::uint32_t>(is.seekg(4, std::ios_base::cur));
		return is.fail() ? 0 : offset;
	}
	
	// Ignore named resource entries.
	const boost::uint32_t entry_size = 4 + 4; // id / string address + offset
	if(is.seekg(nbnames * entry_size, std::ios_base::cur).fail()) {
		return 0;
	}
	
	for(size_t i = 0; i < nbids; i++) {
		
		boost::uint32_t entry_id = util::load<boost::uint32_t>(is);
		boost::uint32_t entry_offset = util::load<boost::uint32_t>(is);
		if(is.fail()) {
			return 0;
		}
		
		if(entry_id == id) {
			return entry_offset;
		}
		
	}
	
	return 0;
}

pe_reader::resource pe_reader::find_resource(std::istream & is, boost::uint32_t name,
                                             boost::uint32_t type,
                                             boost::uint32_t language) {
	
	resource result;
	result.offset = result.size = 0;
	
	header coff;
	if(!coff.load(is)) {
		return result;
	}
	
	section_table sections;
	if(!sections.load(is, coff)) {
		return result;
	}
	
	boost::uint32_t resource_offset = sections.to_file_offset(coff.resource_table_address);
	if(!resource_offset) {
		return result;
	}
	
	is.seekg(resource_offset);
	boost::uint32_t type_offset = find_resource_entry(is, type);
	if(!get_resource_table(type_offset, resource_offset)) {
		return result;
	}
	
	is.seekg(type_offset);
	boost::uint32_t name_offset = find_resource_entry(is, name);
	if(!get_resource_table(name_offset, resource_offset)) {
		return result;
	}
	
	is.seekg(name_offset);
	boost::uint32_t leaf_offset = find_resource_entry(is, language);
	if(!leaf_offset || get_resource_table(leaf_offset, resource_offset)) {
		return result;
	}
	
	// Virtual memory address and size of the resource data.
	is.seekg(leaf_offset);
	boost::uint32_t data_address = util::load<boost::uint32_t>(is);
	boost::uint32_t data_size = util::load<boost::uint32_t>(is);
	// ignore codepage and reserved word
	if(is.fail()) {
		return result;
	}
	
	boost::uint32_t data_offset = sections.to_file_offset(data_address);
	if(!data_offset) {
		return result;
	}
	
	result.offset = data_offset;
	result.size = data_size;
	
	return result;
}

bool pe_reader::get_file_version(std::istream & is) {
	
	resource res = find_resource(is, NameVersionInfo, TypeVersion);
	if(!res) {
		return false;
	}
	
	return skip_to_fixed_file_info<boost::uint16_t>(is, res.offset, 6);
}

} // anonymous namespace

exe_reader::resource exe_reader::find_resource(std::istream & is, boost::uint32_t name,
                                               boost::uint32_t type,
                                               boost::uint32_t language) {
	
	BinaryType bintype = determine_binary_type(is);
	switch(bintype) {
		case OS2Magic: return ne_reader::find_resource(is, name, type);
		case PEMagic:  return pe_reader::find_resource(is, name, type, language);
		default: {
			resource result;
			result.offset = result.size = 0;
			return result;
		}
	}
	
}

boost::uint64_t exe_reader::get_file_version(std::istream & is) {
	
	bool found = false;
	BinaryType bintype = determine_binary_type(is);
	switch(bintype) {
		case OS2Magic: found = ne_reader::get_file_version(is); break;
		case VXDMagic: found = le_reader::get_file_version(is); break;
		case PEMagic:  found = pe_reader::get_file_version(is); break;
		default: break;
	}
	if(!found) {
		return FileVersionUnknown;
	}
	
	boost::uint32_t magic = util::load<boost::uint32_t>(is);
	if(is.fail() || magic != 0xfeef04bd) {
		return FileVersionUnknown;
	}
	
	is.seekg(4, std::ios_base::cur); // skip struct version
	boost::uint32_t file_version_ms = util::load<boost::uint32_t>(is);
	boost::uint32_t file_version_ls = util::load<boost::uint32_t>(is);
	if(is.fail()) {
		return FileVersionUnknown;
	}
	
	return (boost::uint64_t(file_version_ms) << 32) | boost::uint64_t(file_version_ls);
}

} // namespace loader

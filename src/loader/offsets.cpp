/*
 * Copyright (C) 2011-2020 Daniel Scharrer
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

#include "loader/offsets.hpp"

#include <cstring>
#include <limits>

#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>
#include <boost/range/size.hpp>

#include <stddef.h>

#include "crypto/crc32.hpp"
#include "loader/exereader.hpp"
#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/output.hpp"

namespace loader {

namespace {

struct setup_loader_version {
	
	unsigned char magic[12];
	
	// Earliest known version with that ID
	setup::version_constant version;
	
};

const setup_loader_version known_setup_loader_versions[] = {
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '2', 0x87, 'e', 'V', 'x' },    INNO_VERSION(1, 2, 10) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '4', 0x87, 'e', 'V', 'x' },    INNO_VERSION(4, 0,  0) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '5', 0x87, 'e', 'V', 'x' },    INNO_VERSION(4, 0,  3) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '6', 0x87, 'e', 'V', 'x' },    INNO_VERSION(4, 0, 10) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '7', 0x87, 'e', 'V', 'x' },    INNO_VERSION(4, 1,  6) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', 0xcd, 0xe6, 0xd7, '{', 0x0b, '*' }, INNO_VERSION(5, 1,  5) },
	{ { 'n', 'S', '5', 'W', '7', 'd', 'T', 0x83, 0xaa, 0x1b, 0x0f, 'j' }, INNO_VERSION(5, 1,  5) },
};

const int ResourceNameInstaller = 11111;

const boost::uint32_t SetupLoaderHeaderOffset = 0x30;
const boost::uint32_t SetupLoaderHeaderMagic = 0x6f6e6e49; // "Inno"

} // anonymous namespace

bool offsets::load_from_exe_file(std::istream & is) {
	
	is.seekg(SetupLoaderHeaderOffset);
	
	boost::uint32_t magic = util::load<boost::uint32_t>(is);
	if(is.fail() || magic != SetupLoaderHeaderMagic) {
		is.clear();
		return false;
	}
	
	debug("found Inno magic at " << print_hex(SetupLoaderHeaderOffset));
	
	found_magic = true;
	
	boost::uint32_t offset_table_offset = util::load<boost::uint32_t>(is);
	boost::uint32_t not_offset_table_offset = util::load<boost::uint32_t>(is);
	if(is.fail() || offset_table_offset != ~not_offset_table_offset) {
		is.clear();
		debug("header offset checksum: " << print_hex(not_offset_table_offset) << " != ~"
		                                 << print_hex(offset_table_offset));
		return false;
	}
	
	debug("found loader header at " << print_hex(offset_table_offset));
	
	return load_offsets_at(is, offset_table_offset);
}

bool offsets::load_from_exe_resource(std::istream & is) {
	
	exe_reader::resource resource = exe_reader::find_resource(is, ResourceNameInstaller);
	if(!resource) {
		is.clear();
		return false;
	}
	
	debug("found loader header resource at " << print_hex(resource.offset));
	
	found_magic = true;
	
	return load_offsets_at(is, resource.offset);
}

bool offsets::load_offsets_at(std::istream & is, boost::uint32_t pos) {
	
	if(is.seekg(pos).fail()) {
		is.clear();
		debug("could not seek to loader header");
		return false;
	}
	
	char magic[12];
	if(is.read(magic, std::streamsize(sizeof(magic))).fail()) {
		is.clear();
		debug("could not read loader header magic");
		return false;
	}
	
	setup::version_constant version = 0;
	for(size_t i = 0; i < size_t(boost::size(known_setup_loader_versions)); i++) {
		BOOST_STATIC_ASSERT(sizeof(known_setup_loader_versions[i].magic) == sizeof(magic));
		if(!memcmp(magic, known_setup_loader_versions[i].magic, sizeof(magic))) {
			version = known_setup_loader_versions[i].version;
			debug("found loader header magic version " << setup::version(version));
			break;
		}
	}
	if(!version) {
		log_warning << "Unexpected setup loader magic: " << print_hex(magic);
		version = std::numeric_limits<setup::version_constant>::max();
	}
	
	crypto::crc32 checksum;
	checksum.init();
	checksum.update(magic, sizeof(magic));
	
	if(version >= INNO_VERSION(5, 1,  5)) {
		boost::uint32_t revision = checksum.load<boost::uint32_t>(is);
		if(is.fail()) {
			is.clear();
			debug("could not read loader header revision");
			return false;
		} else if(revision != 1) {
			log_warning << "Unexpected setup loader revision: " << revision;
		}
	}
	
	(void)checksum.load<boost::uint32_t>(is);
	exe_offset = checksum.load<boost::uint32_t>(is);
	
	if(version >= INNO_VERSION(4, 1, 6)) {
		exe_compressed_size = 0;
	} else {
		exe_compressed_size = checksum.load<boost::uint32_t>(is);
	}
	
	exe_uncompressed_size = checksum.load<boost::uint32_t>(is);
	
	if(version >= INNO_VERSION(4, 0, 3)) {
		exe_checksum.type = crypto::CRC32;
		exe_checksum.crc32 = checksum.load<boost::uint32_t>(is);
	} else {
		exe_checksum.type = crypto::Adler32;
		exe_checksum.adler32 = checksum.load<boost::uint32_t>(is);
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		message_offset = 0;
	} else {
		message_offset = util::load<boost::uint32_t>(is);
	}
	
	header_offset = checksum.load<boost::uint32_t>(is);
	data_offset = checksum.load<boost::uint32_t>(is);
	
	if(is.fail()) {
		is.clear();
		debug("could not read loader header");
		return false;
	}
	
	if(version >= INNO_VERSION(4, 0, 10)) {
		boost::uint32_t expected = util::load<boost::uint32_t>(is);
		if(is.fail()) {
			is.clear();
			debug("could not read loader header checksum");
			return false;
		}
		if(checksum.finalize() != expected) {
			log_warning << "Setup loader checksum mismatch!";
		}
	}
	
	return true;
}

void offsets::load(std::istream & is) {
	
	found_magic = false;
	
	/*
	 * Try to load the offset table by following a pointer at a constant offset.
	 * This method of storing the offset table is used in versions before 5.1.5
	 */
	if(load_from_exe_file(is)) {
		return;
	}
	
	/*
	 * Try to load an offset table located in a PE/COFF (.exe) resource entry.
	 * This method of storing the offset table was introduced in version 5.1.5
	 */
	if(load_from_exe_resource(is)) {
		return;
	}
	
	/*
	 * If no offset table has been found, this must be an external setup-0.bin file.
	 * In that case, the setup headers start at the beginning of the file.
	 */
	
	exe_compressed_size = exe_uncompressed_size = exe_offset = 0; // No embedded setup exe.
	
	message_offset = 0; // No embedded messages.
	
	header_offset = 0; // Whole file contains just the setup headers.
	
	data_offset = 0; // No embedded setup data.
}

} // namespace loader

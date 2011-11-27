
#include "loader/offsets.hpp"

#include <stdint.h>
#include <cstring>

#include <boost/static_assert.hpp>

#include <stddef.h>

#include "crypto/crc32.hpp"
#include "loader/exereader.hpp"
#include "setup/Version.hpp"
#include "util/load.hpp"
#include "util/log.hpp"

namespace loader {

namespace {

struct setup_loader_version {
	
	unsigned char magic[12];
	
	// Earliest known version with that ID.
	InnoVersionConstant version;
	
};

const setup_loader_version known_setup_loader_versions[] = {
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '2', 0x87, 'e', 'V', 'x' },    INNO_VERSION(1, 2, 10) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '4', 0x87, 'e', 'V', 'x' },    INNO_VERSION(4, 0,  0) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '5', 0x87, 'e', 'V', 'x' },    INNO_VERSION(4, 0,  3) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '6', 0x87, 'e', 'V', 'x' },    INNO_VERSION(4, 0, 10) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', '0', '7', 0x87, 'e', 'V', 'x' },    INNO_VERSION(4, 1,  6) },
	{ { 'r', 'D', 'l', 'P', 't', 'S', 0xcd, 0xe6, 0xd7, '{', 0x0b, '*' }, INNO_VERSION(5, 1,  5) },
};

const int ResourceNameInstaller = 11111;

const uint32_t SetupLoaderHeaderOffset = 0x30;
const uint32_t SetupLoaderHeaderMagic = 0x6f6e6e49;

}; // anonymous namespace

bool offsets::load_from_exe_file(std::istream & is) {
	
	is.seekg(SetupLoaderHeaderOffset);
	
	uint32_t magic = load_number<uint32_t>(is);
	if(is.fail() || magic != SetupLoaderHeaderMagic) {
		is.clear();
		return false;
	}
	
	uint32_t offset_table_offset = load_number<uint32_t>(is);
	uint32_t not_offset_table_offset = load_number<uint32_t>(is);
	if(is.fail() || offset_table_offset != ~not_offset_table_offset) {
		is.clear();
		return false;
	}
	
	return load_offsets_at(is, offset_table_offset);
}

bool offsets::load_from_exe_resource(std::istream & is) {
	
	exe_reader::resource resource = exe_reader::find_resource(is, ResourceNameInstaller);
	if(!resource.offset) {
		is.clear();
		return false;
	}
	
	return load_offsets_at(is, resource.offset);
}

bool offsets::load_offsets_at(std::istream & is, uint32_t pos) {
	
	if(is.seekg(pos).fail()) {
		is.clear();
		return false;
	}
	
	char magic[12];
	if(is.read(magic, ARRAY_SIZE(magic)).fail()) {
		is.clear();
		return false;
	}
	
	InnoVersionConstant version = 0;
	for(size_t i = 0; i < ARRAY_SIZE(known_setup_loader_versions); i++) {
		BOOST_STATIC_ASSERT(ARRAY_SIZE(known_setup_loader_versions[i].magic) == ARRAY_SIZE(magic));
		if(!memcmp(magic, known_setup_loader_versions[i].magic, ARRAY_SIZE(magic))) {
			version = known_setup_loader_versions[i].version;
			break;
		}
	}
	if(!version) {
		return false;
	}
	
	crypto::crc32 checksum;
	checksum.init();
	checksum.update(magic, ARRAY_SIZE(magic));
	
	if(version >= INNO_VERSION(5, 1,  5)) {
		uint32_t revision = checksum.load_number<uint32_t>(is);
		if(is.fail() || revision != 1) {
			is.clear();
			return false;
		}
	}
	
	(void)checksum.load_number<uint32_t>(is);
	exe_offset = checksum.load_number<uint32_t>(is);
	
	if(version >= INNO_VERSION(4, 1, 6)) {
		exe_compressed_size = 0;
	} else {
		exe_compressed_size = checksum.load_number<uint32_t>(is);
	}
	
	exe_uncompressed_size = checksum.load_number<uint32_t>(is);
	
	if(version >= INNO_VERSION(4, 0, 3)) {
		exe_checksum.type = crypto::CRC32;
		exe_checksum.crc32 = checksum.load_number<uint32_t>(is);
	} else {
		exe_checksum.type = crypto::Adler32;
		exe_checksum.adler32 = checksum.load_number<uint32_t>(is);
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		message_offset = 0;
	} else {
		message_offset = load_number<uint32_t>(is);
	}
	
	header_offset = checksum.load_number<uint32_t>(is);
	data_offset = checksum.load_number<uint32_t>(is);
	
	if(is.fail()) {
		is.clear();
		return false;
	}
	
	if(version >= INNO_VERSION(4, 0, 10)) {
		uint32_t expected = load_number<uint32_t>(is);
		if(is.fail()) {
			is.clear();
			return false;
		}
		if(checksum.finalize() != expected) {
			log_error << "[loader] CRC32 mismatch";
			return false;
		}
	}
	
	return true;
}

void offsets::load(std::istream & is) {
	
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

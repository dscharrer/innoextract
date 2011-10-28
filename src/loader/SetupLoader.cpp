
#include <loader/SetupLoader.hpp>

#include <stdint.h>
#include <cstring>

#include <boost/static_assert.hpp>

#include "crypto/CRC32.hpp"
#include "loader/ExeReader.hpp"
#include "setup/Version.hpp"
#include "util/LoadingUtils.hpp"
#include "util/Output.hpp"

namespace {

struct SetupLoaderVersion {
	
	char magic[12];
	
	// Earliest known version with that ID.
	InnoVersionConstant version;
	
};

const SetupLoaderVersion knownSetupLoaderVersions[] = {
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

bool SetupLoader::loadFromExeFile(std::istream & is) {
	
	is.seekg(SetupLoaderHeaderOffset);
	
	uint32_t magic = loadNumber<uint32_t>(is);
	if(is.fail() || magic != SetupLoaderHeaderMagic) {
		is.clear();
		return false;
	}
	
	uint32_t offsetTableOffset = loadNumber<uint32_t>(is);
	uint32_t notOffsetTableOffset = loadNumber<uint32_t>(is);
	if(is.fail() || offsetTableOffset != ~notOffsetTableOffset) {
		is.clear();
		return false;
	}
	
	return loadOffsetsAt(is, offsetTableOffset);
}

bool SetupLoader::loadFromExeResource(std::istream & is) {
	
	ExeReader::Resource resource = ExeReader::findResource(is, ResourceNameInstaller);
	if(!resource.offset) {
		is.clear();
		return false;
	}
	
	return loadOffsetsAt(is, resource.offset);
}

bool SetupLoader::loadOffsetsAt(std::istream & is, size_t pos) {
	
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
	for(size_t i = 0; i < ARRAY_SIZE(knownSetupLoaderVersions); i++) {
		BOOST_STATIC_ASSERT(ARRAY_SIZE(knownSetupLoaderVersions[i].magic) == ARRAY_SIZE(magic));
		if(!memcmp(magic, knownSetupLoaderVersions[i].magic, ARRAY_SIZE(magic))) {
			version = knownSetupLoaderVersions[i].version;
			break;
		}
	}
	if(!version) {
		return false;
	}
	
	Crc32 checksum;
	checksum.init();
	checksum.update(magic, ARRAY_SIZE(magic));
	
	if(version >= INNO_VERSION(5, 1,  5)) {
		uint32_t revision = fromLittleEndian(checksum.process(::load<uint32_t>(is)));
		if(is.fail() || revision != 1) {
			is.clear();
			return false;
		}
	}
	
	totalSize = fromLittleEndian(checksum.process(::load<uint32_t>(is)));
	exeOffset = fromLittleEndian(checksum.process(::load<uint32_t>(is)));
	
	if(version >= INNO_VERSION(4, 1, 6)) {
		exeCompressedSize = 0;
	} else {
		exeCompressedSize = fromLittleEndian(checksum.process(::load<uint32_t>(is)));
	}
	
	exeUncompressedSize = fromLittleEndian(checksum.process(::load<uint32_t>(is)));
	
	if(version >= INNO_VERSION(4, 0, 3)) {
		exeChecksum.type = Checksum::Crc32;
		exeChecksum.crc32 = fromLittleEndian(checksum.process(::load<uint32_t>(is)));
	} else {
		exeChecksum.type = Checksum::Adler32;
		exeChecksum.adler32 = fromLittleEndian(checksum.process(::load<uint32_t>(is)));
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		messageOffset = 0;
	} else {
		messageOffset = loadNumber<uint32_t>(is);
	}
	
	headerOffset = fromLittleEndian(checksum.process(::load<uint32_t>(is)));
	dataOffset = fromLittleEndian(checksum.process(::load<uint32_t>(is)));
	
	if(is.fail()) {
		is.clear();
		return false;
	}
	
	if(version >= INNO_VERSION(4, 0, 10)) {
		uint32_t expected = loadNumber<uint32_t>(is);
		if(is.fail()) {
			is.clear();
			return false;
		}
		if(checksum.finalize() != expected) {
			LogError << "[loader] CRC32 mismatch";
			return false;
		}
	}
	
	return true;
}

void SetupLoader::load(std::istream & is) {
	
	/*
	 * Try to load the offset table by following a pointer at a constant offset.
	 * This method of storing the offset table is used in versions before 5.1.5
	 */
	if(loadFromExeFile(is)) {
		return;
	}
	
	/*
	 * Try to load an offset table located in a COFF/PE (.exe) resource entry.
	 * This method of storing the offset table was introduced in version 5.1.5
	 */
	if(loadFromExeResource(is)) {
		return;
	}
	
	/*
	 * If no offset table has been found, this must be an external setup-0.bin file.
	 * In that case, the setup headers start at the beginning of the file.
	 */
	
	totalSize = is.seekg(0, std::ios_base::end).tellg();
	
	exeOffset = 0; // No embedded setup exe.
	exeCompressedSize = exeUncompressedSize = 0;
	
	messageOffset = 0; // No embedded messages.
	
	headerOffset = 0; // Whole file contains just the setup headers.
	
	dataOffset = 0; // No embedded setup data.
}

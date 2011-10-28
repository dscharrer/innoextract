
#include "stream/BlockReader.hpp"

#include <stdint.h>
#include <istream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include "crypto/CRC32.hpp"
#include "setup/Version.hpp"
#include "stream/BlockFilter.hpp"
#include "stream/LzmaFilter.hpp"
#include "util/Enum.hpp"
#include "util/LoadingUtils.hpp"
#include "util/Utils.hpp"

using std::cout;
using std::endl;

namespace io = boost::iostreams;

namespace {

enum BlockCompression {
	Stored,
	Zlib,
	LZMA1,
};

} // anonymous namespace

NAMED_ENUM(BlockCompression)

ENUM_NAMES(BlockCompression, "Compression", "stored", "zlib", "lzma1")

template <class T>
T loadNumberChecked(std::istream & is, Crc32 & crc) {
	T value = load<T>(is);
	crc.process(value);
	return fromLittleEndian(value);
};

std::istream * BlockReader::get(std::istream & base, const InnoVersion & version) {
	
	uint32_t expectedCrc = loadNumber<uint32_t>(base);
	Crc32 actualCrc;
	actualCrc.init();
	
	uint64_t storedSize;
	BlockCompression compression;
	
	if(version >= INNO_VERSION(4, 0, 9)) {
		storedSize = loadNumberChecked<uint32_t>(base, actualCrc);
		uint8_t compressed = loadNumberChecked<uint8_t>(base, actualCrc);
		compression = compressed ? (version >= INNO_VERSION(4, 1, 6) ? LZMA1 : Zlib) : Stored;
		
	} else {
		
		uint32_t compressedSize = loadNumberChecked<uint32_t>(base, actualCrc);
		uint32_t uncompressedSize = loadNumberChecked<uint32_t>(base, actualCrc);
		
		if(compressedSize == uint32_t(-1)) {
			storedSize = uncompressedSize, compression = Stored;
		} else {
			storedSize = compressedSize, compression = Zlib;
		}
		
		// Add the size of a CRC32 checksum for each 4KiB subblock.
		storedSize += ceildiv(storedSize, 4096) * 4;
	}
	
	if(actualCrc.finalize() != expectedCrc) {
		LogError << "block CRC32 mismatch";
		return NULL;
	}
	
	cout << "[block] size: " << storedSize << "  compression: " << compression << endl;
	
	io::filtering_istream * fis;
	fis = new io::filtering_istream;
	
	switch(compression) {
		case Stored: break;
		case Zlib: fis->push(io::zlib_decompressor(), 8192); break;
		case LZMA1: fis->push(inno_lzma1_decompressor(), 8192); break;
	}
	
	fis->push(inno_block_filter(), 4096);
	
	fis->push(io::restrict(base, 0, storedSize));
	
	return fis;
}

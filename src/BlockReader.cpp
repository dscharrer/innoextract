
#include "BlockReader.hpp"

#include <iostream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <lzma.h>

#include "Utils.hpp"
#include "ChunkFilter.hpp"
#include "LzmaFilter.hpp"
#include "LoadingUtils.hpp"
#include "Enum.hpp"

using std::cout;
using std::endl;

namespace io = boost::iostreams;

enum Compression {
	Stored,
	Zlib,
	LZMA1,
};

NAMED_ENUM(Compression)

ENUM_NAMES(Compression, "Compression", "stored", "zlib", "lzma1")

template <class T>
T loadNumberChecked(std::istream & is, u32 & crc) {
	T value = load<T>(is);
	crc = lzma_crc32(reinterpret_cast<const uint8_t *>(&value), sizeof(value), crc);
	return fromLittleEndian(value);
};

std::istream * BlockReader::get(std::istream & base, const InnoVersion & version) {
	
	u32 expectedCrc = loadNumber<u32>(base);
	u32 actualCrc = 0;
	
	u64 storedSize;
	Compression compression;
	bool chunked;
	
	if(version >= INNO_VERSION(4, 0, 9)) {
		storedSize = loadNumberChecked<u32>(base, actualCrc);
		u8 compressed = loadNumberChecked<u8>(base, actualCrc);
		compression = compressed ? (version >= INNO_VERSION(4, 1, 6) ? LZMA1 : Zlib) : Stored;
		chunked = true;
		
	} else {
		
		u32 compressedSize = loadNumberChecked<u32>(base, actualCrc);
		u32 uncompressedSize = loadNumberChecked<u32>(base, actualCrc);
		
		if(compressedSize == u32(-1)) {
			storedSize = uncompressedSize, compression = Stored;
		} else {
			storedSize = compressedSize, compression = Zlib;
		}
		
		// Add the size of a CRC32 checksum for each 4KiB chunk.
		storedSize += ceildiv(storedSize, 4096) * 4;
	}
	
	if(actualCrc != expectedCrc) {
		error << "block CRC32 mismatch";
		return NULL;
	}
	
	cout << "[block] size: " << storedSize << "  compression: " << compression << endl;
	
	io::filtering_istream * fis;
	fis = new io::filtering_istream;
	
	switch(compression) {
		case Stored: break;
		case Zlib: fis->push(io::zlib_decompressor(), 8192); break;
		case LZMA1: fis->push(inno_lzma_decompressor(), 8192); break;
	}
	
	fis->push(inno_chunk_filter(), 4096);
	
	fis->push(io::restrict(base, 0, storedSize));
	
	return fis;
}

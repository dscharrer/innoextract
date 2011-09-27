
#include "ChunkFilter.hpp"

#include <lzma.h>

#include <zlib.h>

void inno_chunk_filter::checkCrc(u32 expected) const {
	
	u32 actual = lzma_crc32(reinterpret_cast<const uint8_t *>(buffer), length, 0);
	
	if(actual != expected) {
		error << "[chunk] CRC32 mismatch";
		throw std::string("chunk CRC32 mismatch");
	}
	
}

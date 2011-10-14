
#include "stream/BlockFilter.hpp"

#include <lzma.h>

#include <zlib.h>

void inno_block_filter::checkCrc(u32 expected) const {
	
	u32 actual = lzma_crc32(reinterpret_cast<const uint8_t *>(buffer), length, 0);
	
	if(actual != expected) {
		error << "[block] CRC32 mismatch";
		throw std::string("block CRC32 mismatch");
	}
	
}

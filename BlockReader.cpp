
#include "BlockReader.hpp"

#include <iostream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/restrict.hpp>
#include <lzma.h>

#include "Types.h"
#include "Utils.hpp"
#include "ChunkFilter.hpp"
#include "LzmaFilter.hpp"

using std::cout;
using std::endl;

namespace io = boost::iostreams;

#pragma pack(push,1)

struct BlockHeader {
	u32 storedSize; // Total bytes written, including the CRCs
	u8 compressed; // True if data is compressed, False if not
};

#pragma pack(pop)

std::istream * BlockReader::get(std::istream & base) {
	
	u32 crc;
	BlockHeader block;
	if(read(read(base, crc), block).fail()) {
		return NULL;
	}
	
	u32 actual = lzma_crc32(reinterpret_cast<const uint8_t *>(&block), sizeof(block), 0);
	if(crc != actual) {
		return NULL;
	}
	
	cout << "block size: " << block.storedSize << "  compressed: " << int(block.compressed) << endl;
	
	io::filtering_istream * fis;
	fis = new io::filtering_istream;
	
	if(block.compressed) {
		fis->push(inno_lzma_decompressor(), 8192);
	}
	
	fis->push(inno_chunk_filter(), 4096);
	
	fis->push(io::restrict(base, 0, block.storedSize));
	//fis->push(base);
	
	return fis;
}

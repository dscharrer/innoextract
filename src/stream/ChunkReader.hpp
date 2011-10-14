
#ifndef INNOEXTRACT_STREAM_CHUNKREADER_HPP
#define INNOEXTRACT_STREAM_CHUNKREADER_HPP

#include <stddef.h>
#include <istream>
#include <ios>
#include <boost/shared_ptr.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include "util/Types.hpp"

class SliceReader;

class ChunkReader {
	
	u64 storedSize;
	std::string password;
	std::string salt;
	
public:
	
	ChunkReader(u64 _storedSize) : storedSize(_storedSize) { }
	
	u64 chunkDecompressedBytesRead;
	u64 chunkCompressedBytesLeft;
	
	enum CompressionMethod { };
	CompressionMethod chunkCompression;
	
	boost::shared_ptr<std::istream> get(boost::shared_ptr<SliceReader> base);
	
};

/*
 *
 * Open chunk:
 * 
 * seek to dataOffset + firstSliceOffset
 * 4 bytes -> chunkId
 * 
 * 
 * 
 */

#endif // INNOEXTRACT_STREAM_CHUNKREADER_HPP

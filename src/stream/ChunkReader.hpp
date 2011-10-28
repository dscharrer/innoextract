
#ifndef INNOEXTRACT_STREAM_CHUNKREADER_HPP
#define INNOEXTRACT_STREAM_CHUNKREADER_HPP

#include <stddef.h>
#include <stdint.h>
#include <ios>
#include <boost/shared_ptr.hpp>
#include <boost/iostreams/filtering_stream.hpp>

class SliceReader;

class ChunkReader {
	
	uint64_t storedSize;
	std::string password;
	std::string salt;
	
public:
	
	struct Chunk {
		
		size_t firstSlice; //!< Slice where the chunk starts.
		size_t chunkOffset; //!< Offset of the compressed chunk in firstSlice.
		uint64_t chunkSize; //! Total compressed size of the chunk.
		
		bool compressed;
		bool encrypted;
		
		Chunk(size_t firstSlice, size_t chunkOffset, uint64_t chunkSize, bool compressed, bool encrypted);
		
		bool operator<(const Chunk & o) const;
		bool operator==(const Chunk & o) const;
	};
	
	ChunkReader(uint64_t _storedSize) : storedSize(_storedSize) { }
	
	uint64_t chunkDecompressedBytesRead;
	uint64_t chunkCompressedBytesLeft;
	
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

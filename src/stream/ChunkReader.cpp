
#include "ChunkReader.hpp"

#include "SliceReader.hpp"

const char chunkId[4] = { 'z', 'l', 'b', 0x1a };

ChunkReader::Chunk::Chunk(size_t _firstSlice, uint32_t _chunkOffset, uint64_t _chunkSize,
                          bool _compressed, bool _encrypted)
	: firstSlice(_firstSlice), chunkOffset(_chunkOffset), chunkSize(_chunkSize),
	  compressed(_compressed), encrypted(_encrypted) { }

bool ChunkReader::Chunk::operator<(const ChunkReader::Chunk & o) const {
	
	if(firstSlice != o.firstSlice) {
		return (firstSlice < o.firstSlice);
	} else if(chunkOffset != o.chunkOffset) {
		return (chunkOffset < o.chunkOffset);
	} else if(chunkSize != o.chunkSize) {
		return (chunkSize < o.chunkSize);
	} else if(compressed != o.compressed) {
		return (compressed < o.compressed);
	} else if(encrypted != o.encrypted) {
		return (encrypted < o.encrypted);
	}
	
	return false;
}

bool ChunkReader::Chunk::operator==(const ChunkReader::Chunk & o) const {
	
	return (firstSlice == o.firstSlice
	        && chunkOffset == o.chunkOffset
	        && chunkSize == o.chunkSize
	        && compressed == o.compressed
	        && encrypted == o.encrypted);
}

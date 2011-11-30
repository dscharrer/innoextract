
#ifndef INNOEXTRACT_STREAM_CHUNK_HPP
#define INNOEXTRACT_STREAM_CHUNK_HPP

#include <stddef.h>
#include <stdint.h>
#include <ios>

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/chain.hpp>

#include "util/enum.hpp"

namespace stream {

class slice_reader;

struct chunk_error : public std::ios_base::failure {
	
	inline chunk_error(std::string msg) : failure(msg) { }
	
};

enum compression_method {
	Stored,
	Zlib,
	BZip2,
	LZMA1,
	LZMA2,
	UnknownCompression
};

struct chunk {
	
	size_t first_slice; //!< Slice where the chunk starts.
	size_t last_slice;
	
	uint32_t offset;    //!< Offset of the compressed chunk in firstSlice.
	uint64_t size;      //! Total compressed size of the chunk.
	
	compression_method compression;
	bool encrypted;
	
	bool operator<(const chunk & o) const;
	bool operator==(const chunk & o) const;
};

class silce_source;

class chunk_reader {
	
public:
	
	typedef boost::iostreams::chain<boost::iostreams::input> type;
	typedef boost::shared_ptr<type> pointer;
	
	static pointer get(slice_reader & base, const chunk & chunk);
	
};

} // namespace stream

NAMED_ENUM(stream::compression_method)

#endif // INNOEXTRACT_STREAM_CHUNK_HPP

/*
 * Copyright (C) 2011 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

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
	
	chunk_error(std::string msg) : std::ios_base::failure(msg) { }
	
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
	
	static pointer get(slice_reader & base, const ::stream::chunk & chunk);
	
};

} // namespace stream

NAMED_ENUM(stream::compression_method)

#endif // INNOEXTRACT_STREAM_CHUNK_HPP

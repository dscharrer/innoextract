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

#include "chunk.hpp"

#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

#include "version.hpp"
#include "stream/lzma.hpp"
#include "stream/slice.hpp"
#include "util/log.hpp"
#include "util/util.hpp"

namespace io = boost::iostreams;

namespace stream {

static const char chunk_id[4] = { 'z', 'l', 'b', 0x1a };

bool chunk::operator<(const chunk & o) const {
	
	if(first_slice != o.first_slice) {
		return (first_slice < o.first_slice);
	} else if(offset != o.offset) {
		return (offset < o.offset);
	} else if(size != o.size) {
		return (size < o.size);
	} else if(compression != o.compression) {
		return (compression < o.compression);
	} else if(encrypted != o.encrypted) {
		return (encrypted < o.encrypted);
	}
	
	return false;
}

bool chunk::operator==(const chunk & o) const {
	return (first_slice == o.first_slice
	        && offset == o.offset
	        && size == o.size
	        && compression == o.compression
	        && encrypted == o.encrypted);
}

chunk_reader::pointer chunk_reader::get(slice_reader & base, const chunk & chunk) {
	
	if(!base.seek(chunk.first_slice, chunk.offset)) {
		throw chunk_error("error seeking");
	}
	
	char magic[ARRAY_SIZE(chunk_id)];
	if(base.read(magic, 4) != 4 || memcmp(magic, chunk_id, ARRAY_SIZE(chunk_id))) {
		throw chunk_error("bad chunk magic");
	}
	
	pointer result = boost::make_shared<type>();
	
	switch(chunk.compression) {
		case Stored: break;
		case Zlib:   result->push(io::zlib_decompressor(), 8192); break;
		case BZip2:  result->push(io::bzip2_decompressor(), 8192); break;
	#ifdef HAVE_LZMA
		case LZMA1:  result->push(inno_lzma1_decompressor(), 8192); break;
		case LZMA2:  result->push(inno_lzma2_decompressor(), 8192); break;
	#else
		case LZMA1: case LZMA2:
			throw chunk_error("LZMA decompression not supported by this "
			                  + std::string(innoextract_name) + " build");
	#endif
		default: throw chunk_error("unknown compression");
	}
	
	result->push(io::restrict(boost::ref(base), 0, int64_t(chunk.size)));
	
	return result;
}

} // namespace stream

NAMES(stream::compression_method, "Compression Method",
	"stored",
	"zlib",
	"bzip2",
	"lzma1",
	"lzma2",
	"unknown",
)

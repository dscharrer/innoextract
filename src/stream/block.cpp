/*
 * Copyright (C) 2011-2019 Daniel Scharrer
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

#include "stream/block.hpp"

#include <cstring>
#include <string>
#include <istream>
#include <algorithm>

#include <boost/cstdint.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/char_traits.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/make_shared.hpp>

#include "release.hpp"
#include "crypto/crc32.hpp"
#include "setup/version.hpp"
#include "stream/lzma.hpp"
#include "util/endian.hpp"
#include "util/enum.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/math.hpp"

namespace io = boost::iostreams;

namespace stream {

namespace {

enum block_compression {
	Stored,
	Zlib,
	LZMA1,
};

/*!
 * A filter that reads a block of 4096-byte chunks where each chunk is preceeded by
 * a CRC32 checksum. The last chunk can be shorter than 4096 bytes.
 *
 * If chunk checksum is wrong a block_error is thrown before any data of that
 * chunk is returned.
 *
 * block_error is also thrown if there is trailing data: 0 < (total size % (4096 + 4)) < 5
 */
class inno_block_filter : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	inno_block_filter() : pos(0), length(0) { }
	
	template <typename Source>
	bool read_chunk(Source & src) {
		
		char temp[sizeof(boost::uint32_t)];
		std::streamsize temp_size = std::streamsize(sizeof(temp));
		std::streamsize nread = boost::iostreams::read(src, temp, temp_size);
		if(nread == EOF) {
			return false;
		} else if(size_t(nread) != sizeof(temp)) {
			throw block_error("unexpected block end");
		}
		boost::uint32_t block_crc32 = util::little_endian::load<boost::uint32_t>(temp);
		
		length = size_t(boost::iostreams::read(src, buffer, std::streamsize(sizeof(buffer))));
		if(length == size_t(EOF)) {
			throw block_error("unexpected block end");
		}
		
		crypto::crc32 actual;
		actual.init();
		actual.update(buffer, length);
		if(actual.finalize() != block_crc32) {
			throw block_error("block CRC32 mismatch");
		}
		
		pos = 0;
		
		return true;
	}
	
	template <typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n) {
		
		std::streamsize nread = 0;
		while(n) {
			
			if(pos == length && !read_chunk(src)) {
				return nread ? nread : EOF;
			}
			
			std::streamsize size = std::min(n, std::streamsize(length - pos));
			
			std::copy(buffer + pos, buffer + pos + size, dest + nread);
			
			pos += size_t(size), n -= size, nread += size;
		}
		
		return nread;
	}
	
private:
	
	size_t pos; //! Current read position in the buffer.
	size_t length; //! Length of the buffer. This is always 4096 except for the last chunk.
	char buffer[4096];
	
};

} // anonymous namespace

} // namespace stream

NAMED_ENUM(stream::block_compression)

NAMES(stream::block_compression, "Compression",
	"stored",
	"zlib",
	"lzma1",
)

namespace stream {

block_reader::pointer block_reader::get(std::istream & base, const setup::version & version) {
	
	USE_ENUM_NAMES(block_compression)
	
	boost::uint32_t expected_checksum = util::load<boost::uint32_t>(base);
	crypto::crc32 actual_checksum;
	actual_checksum.init();
	
	boost::uint32_t stored_size;
	block_compression compression;
	
	if(version >= INNO_VERSION(4, 0, 9)) {
		
		stored_size = actual_checksum.load<boost::uint32_t>(base);
		boost::uint8_t compressed = actual_checksum.load<boost::uint8_t>(base);
		
		compression = compressed ? (version >= INNO_VERSION(4, 1, 6) ? LZMA1 : Zlib) : Stored;
		
	} else {
		
		boost::uint32_t compressed_size = actual_checksum.load<boost::uint32_t>(base);
		boost::uint32_t uncompressed_size = actual_checksum.load<boost::uint32_t>(base);
		
		if(compressed_size == boost::uint32_t(-1)) {
			stored_size = uncompressed_size, compression = Stored;
		} else {
			stored_size = compressed_size, compression = Zlib;
		}
		
		// Add the size of a CRC32 checksum for each 4KiB subblock.
		stored_size += boost::uint32_t(util::ceildiv<boost::uint64_t>(stored_size, 4096) * 4);
	}
	
	if(actual_checksum.finalize() != expected_checksum) {
		throw block_error("block header CRC32 mismatch");
	}
	
	debug("[block] size: " << stored_size << "  compression: " << compression);
	
	util::unique_ptr<io::filtering_istream>::type fis(new io::filtering_istream);
	
	switch(compression) {
		case Stored: break;
		case Zlib: fis->push(io::zlib_decompressor(), 8192); break;
	#if INNOEXTRACT_HAVE_LZMA
		case LZMA1: fis->push(inno_lzma1_decompressor(), 8192); break;
	#else
		case LZMA1: throw block_error("LZMA decompression not supported by this "
			                  + std::string(innoextract_name) + " build");
	#endif
	}
	
	fis->push(inno_block_filter(), 4096);
	
	fis->push(io::restrict(base, 0, stored_size));
	
	fis->exceptions(std::ios_base::badbit | std::ios_base::failbit);
	
	return pointer(fis.release());
}

} // namespace stream

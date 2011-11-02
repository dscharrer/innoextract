
#ifndef INNOEXTRACT_STREAM_BLOCKFILTER_HPP
#define INNOEXTRACT_STREAM_BLOCKFILTER_HPP

#include <stdint.h>
#include <cstring>
#include <string>
#include <algorithm>
#include <iosfwd>
#include <cassert>

#include <boost/iostreams/char_traits.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/read.hpp>

#include "crypto/CRC32.hpp"
#include "util/Endian.hpp"

struct block_error : public std::ios_base::failure {
	
	inline block_error(std::string msg) : failure(msg) { }
	
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
	
	template<typename Source>
	bool read_chunk(Source & src) {
		
		uint32_t blockCrc32;
		char temp[sizeof(blockCrc32)];
		std::streamsize nread = boost::iostreams::read(src, temp, sizeof(temp));
		if(nread == EOF) {
			return false;
		}	else if(nread != sizeof(temp)) {
			throw block_error("unexpected block end");
		}
		std::memcpy(&blockCrc32, temp, sizeof(blockCrc32));
		blockCrc32 = LittleEndian::byteSwapIfAlien(blockCrc32);
		
		length = boost::iostreams::read(src, buffer, sizeof(buffer));
		if(length == size_t(EOF)) {
			throw block_error("unexpected block end");
		}
		
		Crc32 actual;
		actual.init();
		actual.update(buffer, length);
		if(actual.finalize() != blockCrc32) {
			throw block_error("block CRC32 mismatch");
		}
		
		pos = 0;
		
		return true;
	}
	
	template<typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n) {
		
		size_t read = 0;
		while(n) {
			
			if(pos == length && !read_chunk(src)) {
				return read ? read : EOF;
			}
			
			std::streamsize size = std::min(n, std::streamsize(length - pos));
			
			std::copy(buffer + pos, buffer + pos + size, dest + read);
			
			pos += size, n -= size, read += size;
		}
		
		return read;
	}
	
private:
	
	size_t pos; //! Current read position in the buffer.
	size_t length; //! Length of the buffer. This is always 4096 except for the last chunk.
	char buffer[4096];
	
};

#endif // INNOEXTRACT_STREAM_BLOCKFILTER_HPP


#include "stream/block.hpp"

#include <stdint.h>
#include <cstring>
#include <string>
#include <istream>
#include <algorithm>
#include <cassert>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/char_traits.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/make_shared.hpp>

#include "crypto/crc32.hpp"
#include "setup/Version.hpp"
#include "stream/lzma.hpp"
#include "util/endian.hpp"
#include "util/enum.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/util.hpp"

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
	
	template<typename Source>
	bool read_chunk(Source & src) {
		
		uint32_t block_crc32;
		char temp[sizeof(block_crc32)];
		std::streamsize nread = boost::iostreams::read(src, temp, sizeof(temp));
		if(nread == EOF) {
			return false;
		} else if(nread != sizeof(temp)) {
			throw block_error("unexpected block end");
		}
		std::memcpy(&block_crc32, temp, sizeof(block_crc32));
		block_crc32 = little_endian::byteswap_if_alien(block_crc32);
		
		length = size_t(boost::iostreams::read(src, buffer, sizeof(buffer)));
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
	
	template<typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n) {
		
		std::streamsize read = 0;
		while(n) {
			
			if(pos == length && !read_chunk(src)) {
				return read ? read : EOF;
			}
			
			std::streamsize size = std::min(n, std::streamsize(length - pos));
			
			std::copy(buffer + pos, buffer + pos + size, dest + read);
			
			pos += size_t(size), n -= size, read += size;
		}
		
		return read;
	}
	
private:
	
	size_t pos; //! Current read position in the buffer.
	size_t length; //! Length of the buffer. This is always 4096 except for the last chunk.
	char buffer[4096];
	
};

} // anonymous namespace

} // namespace stream

NAMED_ENUM(stream::block_compression)

ENUM_NAMES(stream::block_compression, "Compression", "stored", "zlib", "lzma1")

namespace stream {

block_reader::pointer block_reader::get(std::istream & base, const InnoVersion & version) {
	
	(void)enum_names<block_compression>::name;
	
	uint32_t expected_checksum = load_number<uint32_t>(base);
	crypto::crc32 actual_checksum;
	actual_checksum.init();
	
	uint32_t stored_size;
	block_compression compression;
	
	if(version >= INNO_VERSION(4, 0, 9)) {
		
		stored_size = actual_checksum.load<little_endian, uint32_t>(base);
		uint8_t compressed = actual_checksum.load<little_endian, uint8_t>(base);
		
		compression = compressed ? (version >= INNO_VERSION(4, 1, 6) ? LZMA1 : Zlib) : Stored;
		
	} else {
		
		uint32_t compressed_size = actual_checksum.load<little_endian, uint32_t>(base);
		uint32_t uncompressed_size = actual_checksum.load<little_endian, uint32_t>(base);
		
		if(compressed_size == uint32_t(-1)) {
			stored_size = uncompressed_size, compression = Stored;
		} else {
			stored_size = compressed_size, compression = Zlib;
		}
		
		// Add the size of a CRC32 checksum for each 4KiB subblock.
		stored_size += uint32_t(ceildiv<uint64_t>(stored_size, 4096) * 4);
	}
	
	if(actual_checksum.finalize() != expected_checksum) {
		throw block_error("block CRC32 mismatch");
	}
	
	debug("[block] size: " << stored_size << "  compression: " << compression);
	
	boost::shared_ptr<io::filtering_istream> fis = boost::make_shared<io::filtering_istream>();
	
	switch(compression) {
		case Stored: break;
		case Zlib: fis->push(io::zlib_decompressor(), 8192); break;
		case LZMA1: fis->push(inno_lzma1_decompressor(), 8192); break;
	}
	
	fis->push(inno_block_filter(), 4096);
	
	fis->push(io::restrict(base, 0, stored_size));
	
	return fis;
}

} // namespace stream

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

#include "chunk.hpp"

#include <cstring>

#include <boost/iostreams/char_traits.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/size.hpp>

#include "release.hpp"
#include "crypto/arc4.hpp"
#include "crypto/checksum.hpp"
#include "crypto/hasher.hpp"
#include "stream/lzma.hpp"
#include "stream/restrict.hpp"
#include "stream/slice.hpp"
#include "util/log.hpp"


namespace io = boost::iostreams;

namespace stream {

namespace {

const char chunk_id[4] = { 'z', 'l', 'b', 0x1a };

#if INNOEXTRACT_HAVE_ARC4

/*!
 * Filter to en-/decrypt files files stored by Inno Setup.
 */
class inno_arc4_crypter : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	inno_arc4_crypter(const char * key, size_t length) {
		
		arc4.init(key, length);
		arc4.discard(1000);
		
	}
	
	template <typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n) {
		
		std::streamsize length = boost::iostreams::read(src, dest, n);
		if(length != EOF) {
			arc4.crypt(dest, dest, size_t(n));
		}
		
		return length;
	}
	
private:
	
	crypto::arc4 arc4;
	
};

#endif // INNOEXTRACT_HAVE_ARC4

} // anonymous namespace

bool chunk::operator<(const chunk & o) const {
	
	if(first_slice != o.first_slice) {
		return (first_slice < o.first_slice);
	} else if(sort_offset != o.sort_offset) {
		return (sort_offset < o.sort_offset);
	} else if(size != o.size) {
		return (size < o.size);
	} else if(compression != o.compression) {
		return (compression < o.compression);
	} else if(encryption != o.encryption) {
		return (encryption < o.encryption);
	}
	
	return false;
}

bool chunk::operator==(const chunk & o) const {
	return (first_slice == o.first_slice
	        && sort_offset == o.sort_offset
	        && size == o.size
	        && compression == o.compression
	        && encryption == o.encryption);
}

chunk_reader::pointer chunk_reader::get(slice_reader & base, const chunk & chunk , const std::string & key) {
	
	if(!base.seek(chunk.first_slice, chunk.offset)) {
		throw chunk_error("could not seek to chunk start");
	}
	
	char magic[sizeof(chunk_id)];
	if(base.read(magic, 4) != 4 || std::memcmp(magic, chunk_id, sizeof(chunk_id)) != 0) {
		throw chunk_error("bad chunk magic");
	}
	
	pointer result(new boost::iostreams::chain<boost::iostreams::input>);
	
	switch(chunk.compression) {
		case Stored: break;
		case Zlib:   result->push(io::zlib_decompressor(), 8192); break;
		case BZip2:  result->push(io::bzip2_decompressor(), 8192); break;
	#if INNOEXTRACT_HAVE_LZMA
		case LZMA1:  result->push(inno_lzma1_decompressor(), 8192); break;
		case LZMA2:  result->push(inno_lzma2_decompressor(), 8192); break;
	#else
		case LZMA1: case LZMA2:
			throw chunk_error("LZMA decompression not supported by this "
			                  + std::string(innoextract_name) + " build");
	#endif
		default: throw chunk_error("unknown chunk compression");
	}
	
	if(chunk.encryption != Plaintext) {
		#if INNOEXTRACT_HAVE_ARC4
		char salt[8];
		if(base.read(salt, 8) != 8) {
			throw chunk_error("could not read chunk salt");
		}
		crypto::hasher hasher(chunk.encryption == ARC4_SHA1 ? crypto::SHA1 : crypto::MD5);
		hasher.update(salt, sizeof(salt));
		hasher.update(key.c_str(), key.length());
		crypto::checksum checksum = hasher.finalize();
		const char * salted_key = chunk.encryption == ARC4_SHA1 ? checksum.sha1 : checksum.md5;
		size_t key_length = chunk.encryption == ARC4_SHA1 ? sizeof(checksum.sha1) : sizeof(checksum.md5);
		result->push(inno_arc4_crypter(salted_key, key_length), 8192);
		#else
		(void)key;
		throw chunk_error("ARC4 decryption not supported");
		#endif
	}
	
	result->push(restrict(base, chunk.size));
	
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

NAMES(stream::encryption_method, "Encryption Method",
	"plaintext",
	"rc4 + md5",
	"rc4 + sha1",
)

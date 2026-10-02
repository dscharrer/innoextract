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
#include "crypto/xchacha20.hpp"
#include "stream/lzma.hpp"
#include "stream/restrict.hpp"
#include "stream/slice.hpp"
#include "util/endian.hpp"
#include "util/log.hpp"


namespace io = boost::iostreams;

namespace stream {

namespace {

const char chunk_id[4] = { 'z', 'l', 'b', 0x1a };

#if INNOEXTRACT_HAVE_DECRYPTION

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
/*!
 * Filter to en-/decrypt files files stored by Inno Setup.
 */
class inno_xchacha20_crypter : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	inno_xchacha20_crypter(const char key[32], const char nonce[24]) {
		
		xchacha20.init(key, nonce);
		
	}
	
	template <typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n) {
		
		std::streamsize length = boost::iostreams::read(src, dest, n);
		if(length != EOF) {
			xchacha20.crypt(dest, dest, size_t(n));
		}
		
		return length;
	}
	
private:
	
	crypto::xchacha20 xchacha20;
	
};

#endif // INNOEXTRACT_HAVE_DECRYPTION

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
	
	switch(chunk.encryption) {
		case Plaintext: break;
		#if INNOEXTRACT_HAVE_DECRYPTION
		case ARC4_MD5: /* fall-through */
		case ARC4_SHA1: {
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
			break;
		}
		case XChaCha20: {
			if(key.length() != crypto::xchacha20::key_size + crypto::xchacha20::nonce_size) {
				throw chunk_error("unexpected key size");
			}
			char nonce[crypto::xchacha20::nonce_size];
			std::memcpy(nonce, key.c_str() + crypto::xchacha20::key_size, crypto::xchacha20::nonce_size);
			util::little_endian::store(util::little_endian::load<boost::uint64_t>(nonce) ^ chunk.offset, nonce);
			util::little_endian::store(util::little_endian::load<boost::uint32_t>(nonce + 8) ^ chunk.first_slice, nonce + 8);
			result->push(inno_xchacha20_crypter(key.c_str(), nonce), 8192);
			break;
		}
		#else
		default: (void)key, throw chunk_error("XChaCha20 decryption not supported in this build");
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
	"xchacha20",
)

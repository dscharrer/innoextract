/*
 * Copyright (C) 2024 Daniel Scharrer
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

/*!
 * \file
 *
 * PBKDF2 key derivationroutines.
 */
#ifndef INNOEXTRACT_CRYPTO_PBKDF2_HPP
#define INNOEXTRACT_CRYPTO_PBKDF2_HPP

#include "configure.hpp"

#if INNOEXTRACT_HAVE_DECRYPTION

#include <stddef.h>

#include <algorithm>
#include <cstring>

#include <boost/cstdint.hpp>

namespace crypto {
	
//! HMAC calculation
template <typename T>
struct hmac {
	
	enum constants {
		block_size = T::block_size,
		hash_size = T::hash_size * sizeof(typename T::hash_word),
	};
	
	void init(const char ikey[block_size]) {
		inner.init();
		inner.update(ikey, block_size);
	}
	
	void update(const char * data, size_t length) {
		inner.update(data, length);
	}
	
	void finalize(const char okey[block_size], char mac[hash_size]) {
		
		char buffer[hash_size];
		inner.finalize(buffer);
		
		T outer;
		outer.init();
		outer.update(okey, block_size);
		outer.update(buffer, hash_size);
		outer.finalize(mac);
		
	}
	
	static void prepare_key(const char * password, size_t length,
	                        char ikey[block_size], char okey[block_size]) {
		
		if(length > block_size) {
			T hash;
			hash.init();
			hash.update(password, length);
			hash.finalize(ikey);
			length = hash_size;
		} else {
			std::memcpy(ikey, password, length);
		}
		std::memset(ikey + length, 0, block_size - length);
		
		for(size_t i = 0; i < block_size; i++) {
			okey[i] = char(boost::uint8_t(ikey[i]) ^ boost::uint8_t(0x5c));
			ikey[i] = char(boost::uint8_t(ikey[i]) ^ boost::uint8_t(0x36));
		}
		
	}
	
private:
	
	T inner;
	
};

//! PBKDF2 key derivation calculation
template <typename T>
struct pbkdf2 {
	
	typedef hmac<T> hmac_t;
	enum constants {
		block_size = hmac_t::block_size,
		hash_size = hmac_t::hash_size,
	};
	
	static void derive(const char * password, size_t password_length, const char * salt, size_t salt_length,
	                   size_t iterations, char * key, size_t key_length) {
		
		char ikey[block_size], okey[block_size];
		hmac_t::prepare_key(password, password_length, ikey, okey);
		
		for(size_t block = 1; key_length > 0; block++) {
			
			char u[hash_size];
			{
				char b[4] = { char(block >> 24), char(block >> 16), char(block >> 8), char(block) };
				hmac_t mac;
				mac.init(ikey);
				mac.update(salt, salt_length);
				mac.update(b, sizeof(b));
				mac.finalize(okey, u);
			}
			char f[hash_size];
			std::memcpy(f, u, hash_size);
			
			for(size_t i = 1; i < iterations; i++) {
				hmac_t mac;
				mac.init(ikey);
				mac.update(u, hash_size);
				mac.finalize(okey, u);
				for(size_t j = 0; j < hash_size; j++) {
					f[j] = char(boost::uint8_t(f[j]) ^ boost::uint8_t(u[j]));
				}
			}
			
			size_t n = std::min(size_t(hash_size), key_length);
			std::memcpy(key, f, n);
			key += n;
			key_length -= n;
		}
		
	}
	
};

} // namespace crypto

#endif // INNOEXTRACT_HAVE_DECRYPTION

#endif // INNOEXTRACT_CRYPTO_PBKDF2_HPP


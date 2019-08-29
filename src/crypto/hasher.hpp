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

/*!
 * \file
 *
 * Utility to hash data with a configurable hash function.
 */
#ifndef INNOEXTRACT_CRYPTO_HASHER_HPP
#define INNOEXTRACT_CRYPTO_HASHER_HPP

#include "crypto/adler32.hpp"
#include "crypto/checksum.hpp"
#include "crypto/crc32.hpp"
#include "crypto/md5.hpp"
#include "crypto/sha1.hpp"
#include "util/enum.hpp"

struct checksum_uninitialized_error { };

namespace crypto {

class hasher : checksum_base<hasher> {
	
public:
	
	explicit hasher(checksum_type type);
	
	void update(const char * data, size_t size);
	
	checksum finalize();
	
private:
	
	checksum_type active_type;
	
	union {
		crypto::adler32 adler32;
		crypto::crc32 crc32;
		crypto::md5 md5;
		crypto::sha1 sha1;
	};
	
};

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_HASHER_HPP;

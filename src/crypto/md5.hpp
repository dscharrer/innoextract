/*
 * Copyright (C) 2011-2015 Daniel Scharrer
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
 * MD5 hashing routines.
 */
#ifndef INNOEXTRACT_CRYPTO_MD5_HPP
#define INNOEXTRACT_CRYPTO_MD5_HPP

#include <boost/cstdint.hpp>

#include "crypto/iteratedhash.hpp"
#include "util/endian.hpp"

namespace crypto {

class md5_transform {
	
public:
	
	typedef boost::uint32_t hash_word;
	typedef util::little_endian byte_order;
	static const size_t offset = 0;
	static const size_t block_size = 64;
	static const size_t hash_size = 16;
	
	static void init(hash_word * state);
	
	static void transform(hash_word * state, const hash_word * data);
	
};

typedef iterated_hash<md5_transform> md5;

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_MD5_HPP

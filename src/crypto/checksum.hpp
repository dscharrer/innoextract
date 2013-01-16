/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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

#ifndef INNOEXTRACT_CRYPTO_CHECKSUM_HPP
#define INNOEXTRACT_CRYPTO_CHECKSUM_HPP

#include <stdint.h>
#include <cstring>
#include <iosfwd>
#include <istream>

#include "util/endian.hpp"
#include "util/enum.hpp"
#include "util/types.hpp"

namespace crypto {

enum checksum_type {
	Adler32,
	CRC32,
	MD5,
	SHA1,
};

struct checksum {
	
	union {
		uint32_t adler32;
		uint32_t crc32;
		char md5[16];
		char sha1[20];
	};
	
	checksum_type type;
	
	bool operator==(const checksum & other) const;
	bool operator!=(const checksum & other) const { return !(*this == other); }
	
};

template <class Impl>
class checksum_base : public static_polymorphic<Impl> {
	
public:
	
	/*!
	 * Load the data and process it.
	 * Data is processed as-is and then converted according to Endianness.
	 */
	template <class T, class Endianness>
	T load_number(std::istream & is) {
		T result;
		char buf[sizeof(result)];
		is.read(buf, sizeof(buf));
		this->impl().update(buf, sizeof(buf));
		std::memcpy(&result, buf, sizeof(result));
		return little_endian::byteswap_if_alien(result);
	}
	
	/*!
	 * Load the data and process it.
	 * Data is processed as-is and then converted if the host endianness is not little_endian.
	 */
	template <class T>
	T load_number(std::istream & is) {
		return load_number<T, little_endian>(is);
	}
	
};

} // namespace crypto

NAMED_ENUM(crypto::checksum_type)

std::ostream & operator<<(std::ostream & os, const crypto::checksum & checksum);

#endif // INNOEXTRACT_CRYPTO_CHECKSUM_HPP

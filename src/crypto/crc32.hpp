/*
 * Copyright (C) 2011-2014 Daniel Scharrer
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
 * CRC32 checksum routines.
 */
#ifndef INNOEXTRACT_CRYPTO_CRC32_HPP
#define INNOEXTRACT_CRYPTO_CRC32_HPP

#include <boost/cstdint.hpp>

#include "crypto/checksum.hpp"

namespace crypto {

//! CRC32 checksum calculation
struct crc32 : public checksum_base<crc32> {
	
	void init() { crc = CRC32_NEGL; }
	
	void update(const char * data, size_t length);
	
	boost::uint32_t finalize() const { return crc ^ CRC32_NEGL; }
	
private:
	
	static const boost::uint32_t CRC32_NEGL = 0xffffffffl;
	
	boost::uint32_t crc;
};

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_CRC32_HPP

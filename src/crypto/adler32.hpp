/*
 * Copyright (C) 2011 Daniel Scharrer
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

#ifndef INNOEXTRACT_CRYPTO_ADLER32_HPP
#define INNOEXTRACT_CRYPTO_ADLER32_HPP

#include <stddef.h>
#include <stdint.h>

#include "crypto/checksum.hpp"

namespace crypto {

//! ADLER-32 checksum calculations
struct adler32 : public checksum_base<adler32> {
	
	void init() { s1 = 1, s2 = 0; }
	
	void update(const char * data, size_t length);
	
	uint32_t finalize() const { return (uint32_t(s2) << 16) | s1; }
	
private:
	
	uint16_t s1, s2;
};

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_ADLER32_HPP

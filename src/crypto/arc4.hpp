/*
 * Copyright (C) 2018 Daniel Scharrer
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
 * Alledged RC4 en-/decryption routines.
 */
#ifndef INNOEXTRACT_CRYPTO_ARC4_HPP
#define INNOEXTRACT_CRYPTO_ARC4_HPP

#include <stddef.h>

#include <boost/cstdint.hpp>

#include "configure.hpp"

#if INNOEXTRACT_HAVE_ARC4

namespace crypto {

//! Alledged RC4 en-/decryption calculation
struct arc4 {
	
	void init(const char * key, size_t length);
	
	void discard(size_t length);
	
	void crypt(const char * in, char * out, size_t length);
	
private:
	
	void update();
	
	boost::uint8_t state[256];
	size_t a, b;
	
};

} // namespace crypto

#endif // INNOEXTRACT_HAVE_ARC4

#endif // INNOEXTRACT_CRYPTO_ARC4_HPP


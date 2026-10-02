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
 * ChaCha20 en-/decryption routines.
 */
#ifndef INNOEXTRACT_CRYPTO_XCHACHA20_HPP
#define INNOEXTRACT_CRYPTO_XCHACHA20_HPP

#include <stddef.h>

#include <boost/cstdint.hpp>

#include "configure.hpp"

#if INNOEXTRACT_HAVE_DECRYPTION

namespace crypto {

//! ChaCha20 en-/decryption calculation
struct xchacha20 {
	
	enum constants {
		key_size = 32,
		nonce_size = 24,
	};
	
	typedef boost::uint32_t word;
	
	void init(const char key[key_size], const char nonce[nonce_size]);
	
	void discard(size_t length);
	
	void crypt(const char * in, char * out, size_t length);
	
private:
	
	void update();
	
	static void derive_subkey(const char key[key_size], const char nonce[16], char subkey[key_size]);
	
	static void init_state(word state[16], const char key[key_size]);
	
	static void run_rounds(word keystream[16]);
	
	static void increment_count(word state[16], size_t increment = 1);
	
	word state[16];
	word keystream[16];
	boost::uint8_t pos;
	
	#ifdef INNOEXTRACT_BUILD_TESTS
	friend struct xchacha20_test;
	#endif
};

} // namespace crypto

#endif // INNOEXTRACT_HAVE_DECRYPTION

#endif // INNOEXTRACT_CRYPTO_XCHACHA20_HPP

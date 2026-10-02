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

// Based on Inno Setup's PBKDF2.pas

#include "crypto/pbkdf2.hpp"

#include <cstring>

#include "crypto/sha1.hpp"
#include "crypto/sha256.hpp"
#include "util/test.hpp"

namespace crypto {

INNOEXTRACT_TEST(pbkdf2,
	
	// Testcase from Inno Setup's TestPBKDF2SHA256
	// which is based on https://stackoverflow.com/a/5136918/301485 and https://en.wikipedia.org/wiki/PBKDF2
	
	char buffer[40];
	
	const char * password0 = "password";
	const char * salt0 = "salt";
	const boost::uint8_t key0[] = {
		0x12, 0x0f, 0xb6, 0xcf, 0xfc, 0xf8, 0xb3, 0x2c, 0x43, 0xe7, 0x22, 0x52, 0x56, 0xc4, 0xf8, 0x37,
		0xa8, 0x65, 0x48, 0xc9, 0x2c, 0xcc, 0x35, 0x48, 0x08, 0x05, 0x98, 0x7c, 0xb7, 0x0b, 0xe1, 0x7b
	};
	pbkdf2<sha256>::derive(password0, std::strlen(password0), salt0, std::strlen(salt0), 1, buffer, 32);
	test_equals("sha256.single", buffer, key0, sizeof(key0));
	const boost::uint8_t key1[] = {
		0xc5, 0xe4, 0x78, 0xd5, 0x92, 0x88, 0xc8, 0x41, 0xaa, 0x53, 0x0d, 0xb6, 0x84, 0x5c, 0x4c, 0x8d,
		0x96, 0x28, 0x93, 0xa0, 0x01, 0xce, 0x4e, 0x11, 0xa4, 0x96, 0x38, 0x73, 0xaa, 0x98, 0x13, 0x4a
	};
	pbkdf2<sha256>::derive(password0, std::strlen(password0), salt0, std::strlen(salt0), 4096, buffer, 32);
	test_equals("sha256.multiple", buffer, key1, sizeof(key1));
	
	const char * password1 = "passwordPASSWORDpassword";
	const char * salt1 = "saltSALTsaltSALTsaltSALTsaltSALTsalt";
	const boost::uint8_t key2[] = {
		0x34, 0x8c, 0x89, 0xdb, 0xcb, 0xd3, 0x2b, 0x2f, 0x32, 0xd8,
		0x14, 0xb8, 0x11, 0x6e, 0x84, 0xcf, 0x2b, 0x17, 0x34, 0x7e,
		0xbc, 0x18, 0x00, 0x18, 0x1c, 0x4e, 0x2a, 0x1f, 0xb8, 0xdd,
		0x53, 0xe1, 0xc6, 0x35, 0x51, 0x8c, 0x7d, 0xac, 0x47, 0xe9
	};
	pbkdf2<sha256>::derive(password1, std::strlen(password1), salt1, std::strlen(salt1), 4096, buffer, 40);
	test_equals("sha256.longkey", buffer, key2, sizeof(key2));
	
	const char * password2 = "pass\0word";
	const char * salt2 = "sa\0lt";
	const boost::uint8_t key3[] = {
		0x89, 0xb6, 0x9d, 0x05, 0x16, 0xf8, 0x29, 0x89, 0x3c, 0x69, 0x62, 0x26, 0x65, 0x0a, 0x86, 0x87
	};
	pbkdf2<sha256>::derive(password2, 9, salt2, 5, 4096, buffer, 16);
	test_equals("sha256.evilpassword", buffer, key3, sizeof(key3));
	
	const char * password3 = "plnlrtfpijpuhqylxbgqiiyipieyxvfsavzgxbbcfusqkozwpngsyejqlmjsytrmd";
	const boost::uint8_t  salt3[] = {
		0xa0, 0x09, 0xc1, 0xa4, 0x85, 0x91, 0x2c, 0x6a, 0xe6, 0x30, 0xd3, 0xe7, 0x44, 0x24, 0x0b, 0x04
	};
	const boost::uint8_t key4[] = {
		0x28, 0x86, 0x9b, 0x5f, 0x31, 0xae, 0x29, 0x23, 0x6f, 0x16, 0x4c, 0x5c, 0xb3, 0x3e, 0x2e, 0x3b
	};
	pbkdf2<sha256>::derive(password3, std::strlen(password3), reinterpret_cast<const char *>(salt3),
	                       sizeof(salt3), 1000, buffer, 16);
	test_equals("sha256.longpassword", buffer, key4, sizeof(key4));
	
)

} // namespace crypto

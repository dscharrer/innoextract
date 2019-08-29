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

#include "crypto/hasher.hpp"

namespace crypto {

hasher::hasher(checksum_type type) : active_type(type) {
	
	switch(active_type) {
		case crypto::None: break;
		case crypto::Adler32: adler32.init(); break;
		case crypto::CRC32: crc32.init(); break;
		case crypto::MD5: md5.init(); break;
		case crypto::SHA1: sha1.init(); break;
	};
	
}

void hasher::update(const char * data, size_t size) {
	
	switch(active_type) {
		case crypto::None: break;
		case crypto::Adler32: adler32.update(data, size); break;
		case crypto::CRC32: crc32.update(data, size); break;
		case crypto::MD5: md5.update(data, size); break;
		case crypto::SHA1: sha1.update(data, size); break;
	};
	
}

checksum hasher::finalize() {
	
	checksum result;
	
	result.type = active_type;
	
	switch(active_type) {
		case crypto::None: break;
		case crypto::Adler32: result.adler32 = adler32.finalize(); break;
		case crypto::CRC32: result.crc32 = crc32.finalize(); break;
		case crypto::MD5: md5.finalize(result.md5); break;
		case crypto::SHA1: sha1.finalize(result.sha1); break;
	};
	
	return result;
}

} // namespace crypto

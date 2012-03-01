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

#include "crypto/checksum.hpp"

#include <ios>
#include <cstring>
#include <ostream>
#include <iomanip>

namespace crypto {

bool checksum::operator==(const checksum & o) const {
	
	if(o.type != type) {
		return false;
	}
	
	switch(type) {
		case Adler32: return (adler32 == o.adler32);
		case CRC32: return (crc32 == o.crc32);
		case MD5: return !memcmp(md5, o.md5, ARRAY_SIZE(md5));
		case SHA1: return !memcmp(sha1, o.sha1, ARRAY_SIZE(sha1));
		default: return false;
	};
}

} // namespace crypto

NAMES(crypto::checksum_type, "Checksum Type",
	"Adler32",
	"CRC32",
	"MD5",
	"Sha1",
)

std::ostream & operator<<(std::ostream & os, const crypto::checksum & checksum) {
	
	std::ios_base::fmtflags old_fmtflags = os.flags();
	
	os << checksum.type << ' ';
	
	switch(checksum.type) {
		case crypto::Adler32: {
			os << "0x" << std::hex << std::setw(8) << checksum.adler32;
			break;
		}
		case crypto::CRC32: {
			os << "0x" << std::hex << std::setw(8) << checksum.crc32;
			break;
		}
		case crypto::MD5: {
			for(size_t i = 0; i < ARRAY_SIZE(checksum.md5); i++) {
				os << std::setfill('0') << std::hex << std::setw(2) << int(uint8_t(checksum.md5[i]));
			}
			break;
		}
		case crypto::SHA1: {
			for(size_t i = 0; i < ARRAY_SIZE(checksum.sha1); i++) {
				os << std::setfill('0') << std::hex << std::setw(2) << int(uint8_t(checksum.sha1[i]));
			}
			break;
		}
	}
	
	os.setf(old_fmtflags, std::ios_base::basefield);
	
	return os;
}

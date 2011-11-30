
#include "crypto/checksum.hpp"

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

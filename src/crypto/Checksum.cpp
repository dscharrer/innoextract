
#include "crypto/Checksum.hpp"

#include <cstring>

#include "util/output.hpp"

ENUM_NAMES(Checksum::Type, "Checksum Type",
	"Adler32",
	"CRC32",
	"MD5",
	"Sha1",
)

bool Checksum::operator==(const Checksum & o) const {
	
	if(o.type != type) {
		return false;
	}
	
	switch(type) {
		case Adler32: return (adler32 == o.adler32);
		case Crc32: return (crc32 == o.crc32);
		case MD5: return !memcmp(md5, o.md5, ARRAY_SIZE(md5));
		case Sha1: return !memcmp(sha1, o.sha1, ARRAY_SIZE(sha1));
	};
}

std::ostream & operator<<(std::ostream & os, const Checksum & checksum) {
	
	std::ios_base::fmtflags old = os.flags();
	
	os << checksum.type << ' ';
	
	switch(checksum.type) {
		case Checksum::Adler32: {
			os << print_hex(checksum.adler32);
			break;
		}
		case Checksum::Crc32: {
			os << print_hex(checksum.crc32);
			break;
		}
		case Checksum::MD5: {
			for(size_t i = 0; i < ARRAY_SIZE(checksum.md5); i++) {
				os << std::setfill('0') << std::hex << std::setw(2) << int(uint8_t(checksum.md5[i]));
			}
			break;
		}
		case Checksum::Sha1: {
			for(size_t i = 0; i < ARRAY_SIZE(checksum.sha1); i++) {
				os << std::setfill('0') << std::hex << std::setw(2) << int(uint8_t(checksum.sha1[i]));
			}
			break;
		}
	}
	
	os.setf(old, std::ios_base::basefield);
	
	return os;
}

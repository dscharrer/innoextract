
#include "crypto/Checksum.hpp"

#include <cstring>

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

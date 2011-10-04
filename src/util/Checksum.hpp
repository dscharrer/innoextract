
#ifndef INNOEXTRACT_UTIL_CHECKSUM_HPP
#define INNOEXTRACT_UTIL_CHECKSUM_HPP

#include "util/Enum.hpp"
#include "util/Types.hpp"

struct Checksum {
	
	typedef char MD5Digest[16];
	typedef char SHA1Digest[20];
	
	enum Type {
		Adler32,
		Crc32,
		MD5,
		Sha1,
	};
	
	union {
		u32 adler32;
		u32 crc32;
		MD5Digest md5;
		SHA1Digest sha1;
	};
	
	Type type;
	
};

NAMED_ENUM(Checksum::Type)

#endif // INNOEXTRACT_UTIL_CHECKSUM_HPP;

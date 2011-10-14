
#include "util/Checksum.hpp"

#include <cstring>

#include <lzma.h>
#include <zlib.h>

ENUM_NAMES(Checksum::Type, "Checksum Type",
	"Adler32",
	"CRC32",
	"MD5",
	"Sha1",
)

void Checksum::init(Checksum::Type newType) {
	
	type = newType;
	
	switch(type) {
		case Adler32: adler32 = ::adler32(0l, Z_NULL, 0); break;
		case Crc32: crc32 = 0; break;
		case MD5: /* TODO */ break;
		case Sha1: /* TODO */ break;
	};
}

void Checksum::process(const void * data, size_t size) {
	
	switch(type) {
		case Adler32: adler32 = ::adler32(adler32, reinterpret_cast<const Bytef *>(data), size); break;
		case Crc32: crc32 = lzma_crc32(reinterpret_cast<const uint8_t *>(data), size, crc32); break;
		case MD5: /* TODO */ break;
		case Sha1: /* TODO */ break;
	};
}

void Checksum::finalize() {
	
	switch(type) {
		case Adler32: break;
		case Crc32: break;
		case MD5: /* TODO */ break;
		case Sha1: /* TODO */ break;
	};
}

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

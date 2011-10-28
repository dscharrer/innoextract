
#include "crypto/Hasher.hpp"

void Hasher::init(Checksum::Type newType) {
	
	type = newType;
	
	switch(type) {
		case Checksum::Adler32: adler32.init(); break;
		case Checksum::Crc32: crc32.init(); break;
		case Checksum::MD5: md5.init(); break;
		case Checksum::Sha1: sha1.init(); break;
	};
}

void Hasher::update(const char * data, size_t size) {
	
	switch(type) {
		case Checksum::Adler32: adler32.update(data, size); break;
		case Checksum::Crc32: crc32.update(data, size); break;
		case Checksum::MD5: md5.update(data, size); break;
		case Checksum::Sha1: sha1.update(data, size); break;
	};
}

void Hasher::finalize(Checksum & result) {
	
	result.type = type;
	
	switch(type) {
		case Checksum::Adler32: result.adler32 = adler32.finalize(); break;
		case Checksum::Crc32: result.crc32 = crc32.finalize(); break;
		case Checksum::MD5: md5.finalize(result.md5); break;
		case Checksum::Sha1: sha1.finalize(result.sha1); break;
	};
}

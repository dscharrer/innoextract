
#include "crypto/hasher.hpp"

namespace crypto {

hasher::hasher(const hasher & o) {
	
	type = o.type;
	
	switch(type) {
		case crypto::Adler32: adler32 = o.adler32; break;
		case crypto::CRC32: crc32 = o.crc32; break;
		case crypto::MD5: md5 = o.md5; break;
		case crypto::SHA1: sha1 = o.sha1; break;
	};
}

void hasher::init(checksum_type newType) {
	
	type = newType;
	
	switch(type) {
		case crypto::Adler32: adler32.init(); break;
		case crypto::CRC32: crc32.init(); break;
		case crypto::MD5: md5.init(); break;
		case crypto::SHA1: sha1.init(); break;
	};
}

void hasher::update(const char * data, size_t size) {
	
	switch(type) {
		case crypto::Adler32: adler32.update(data, size); break;
		case crypto::CRC32: crc32.update(data, size); break;
		case crypto::MD5: md5.update(data, size); break;
		case crypto::SHA1: sha1.update(data, size); break;
	};
}

checksum hasher::finalize() {
	
	checksum result;
	
	result.type = type;
	
	switch(type) {
		case crypto::Adler32: result.adler32 = adler32.finalize(); break;
		case crypto::CRC32: result.crc32 = crc32.finalize(); break;
		case crypto::MD5: md5.finalize(result.md5); break;
		case crypto::SHA1: sha1.finalize(result.sha1); break;
	};
	
	return result;
}

} // namespace crypto

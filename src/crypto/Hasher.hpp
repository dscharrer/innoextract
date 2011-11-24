
#ifndef INNOEXTRACT_CRYPTO_HASHER_HPP
#define INNOEXTRACT_CRYPTO_HASHER_HPP

#include "crypto/Adler-32.hpp"
#include "crypto/Checksum.hpp"
#include "crypto/CRC32.hpp"
#include "crypto/MD5.hpp"
#include "crypto/SHA-1.hpp"
#include "util/enum.hpp"

struct checksum_uninitialized_error { };

class Hasher : ChecksumBase<Hasher> {
	
public:
	
	inline Hasher() { }
	inline Hasher(Checksum::Type type) { init(type); }
	
	void init(Checksum::Type type);
	
	void update(const char * data, size_t size);
	
	void finalize(Checksum & result);
	
private:
	
	Checksum::Type type;
	
	union {
		Adler32 adler32;
		Crc32 crc32;
		Md5 md5;
		Sha1 sha1;
	};
	
};

#endif // INNOEXTRACT_CRYPTO_HASHER_HPP;

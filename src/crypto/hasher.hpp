
#ifndef INNOEXTRACT_CRYPTO_HASHER_HPP
#define INNOEXTRACT_CRYPTO_HASHER_HPP

#include "crypto/adler32.hpp"
#include "crypto/checksum.hpp"
#include "crypto/crc32.hpp"
#include "crypto/md5.hpp"
#include "crypto/sha1.hpp"
#include "util/enum.hpp"

struct checksum_uninitialized_error { };

namespace crypto {

class hasher : checksum_base<hasher> {
	
public:
	
	inline hasher() { }
	inline hasher(checksum_type type) { init(type); }
	hasher(const hasher & o);
	
	void init(checksum_type type);
	
	void update(const char * data, size_t size);
	
	checksum finalize();
	
private:
	
	checksum_type type;
	
	union {
		crypto::adler32 adler32;
		crypto::crc32 crc32;
		crypto::md5 md5;
		crypto::sha1 sha1;
	};
	
};

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_HASHER_HPP;


#ifndef INNOEXTRACT_CRYPTO_MD5_HPP
#define INNOEXTRACT_CRYPTO_MD5_HPP

#include <stdint.h>

#include "crypto/iteratedhash.hpp"
#include "util/endian.hpp"

namespace crypto {

class md5_transform {
	
public:
	
	typedef uint32_t hash_word;
	typedef little_endian byte_order;
	static const size_t block_size = 64;
	static const size_t hash_size = 16;
	
	static void init(hash_word * state);
	
	static void transform(hash_word * digest, const hash_word * data);
};

typedef iterated_hash<md5_transform> md5;

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_MD5_HPP

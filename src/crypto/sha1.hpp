
#ifndef INNOEXTRACT_CRYPTO_SHA1_HPP
#define INNOEXTRACT_CRYPTO_SHA1_HPP

#include <stdint.h>

#include "crypto/iteratedhash.hpp"
#include "util/endian.hpp"

namespace crypto {

class sha1_transform {
	
public:
	
	typedef uint32_t hash_word;
	typedef big_endian byte_order;
	static const size_t block_size = 64;
	static const size_t hash_size = 20;
	
	static void init(hash_word * state);
	
	static void transform(hash_word * digest, const hash_word * data);
};

typedef iterated_hash<sha1_transform> sha1;

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_SHA1_HPP

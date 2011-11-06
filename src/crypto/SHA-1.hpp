
#ifndef INNOEXTRACT_CRYPTO_SHA1_HPP
#define INNOEXTRACT_CRYPTO_SHA1_HPP

#include <stdint.h>
#include "crypto/IteratedHash.hpp"

class Sha1Transform {
	
public:
	
	typedef uint32_t HashWord;
	typedef BigEndian ByteOrder;
	static const size_t BlockSize = 64;
	static const size_t HashSize = 20;
	
	static void init(HashWord * state);
	
	static void transform(HashWord * digest, const HashWord * data);
};

typedef IteratedHash<Sha1Transform> Sha1;

#endif // INNOEXTRACT_CRYPTO_SHA1_HPP

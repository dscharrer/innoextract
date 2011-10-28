
#ifndef INNOEXTRACT_CRYPTO_MD5_HPP
#define INNOEXTRACT_CRYPTO_MD5_HPP

#include <stdint.h>
#include "crypto/IteratedHash.hpp"

class Md5Transform {
	
public:
	
	typedef uint32_t HashWord;
	typedef LittleEndian ByteOrder;
	static const size_t BlockSize = 64;
	static const size_t HashSize = 16;
	
	static void init(HashWord * state);
	
	static void transform(HashWord * digest, const HashWord * data);
};

typedef IteratedHash<Md5Transform> Md5;

#endif // INNOEXTRACT_CRYPTO_MD5_HPP

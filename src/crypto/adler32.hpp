
#ifndef INNOEXTRACT_CRYPTO_ADLER32_HPP
#define INNOEXTRACT_CRYPTO_ADLER32_HPP

#include <stddef.h>
#include <stdint.h>

#include "crypto/checksum.hpp"

namespace crypto {

//! ADLER-32 checksum calculations
struct adler32 : public checksum_base<adler32> {
	
	void init() { s1 = 1, s2 = 0; }
	
	void update(const char * data, size_t length);
	
	inline uint32_t finalize() const { return (uint32_t(s2) << 16) | s1; }
	
private:
	
	uint16_t s1, s2;
};

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_ADLER32_HPP

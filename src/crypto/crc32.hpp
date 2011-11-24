
#ifndef INNOEXTRACT_CRYPTO_CRC32_HPP
#define INNOEXTRACT_CRYPTO_CRC32_HPP

#include <stdint.h>

#include "crypto/checksum.hpp"

namespace crypto {

//! CRC32 checksum calculation
struct crc32 : public checksum_base<crc32> {
	
	inline void init() { crc = CRC32_NEGL; }
	
	void update(const char * data, size_t length);
	
	inline uint32_t finalize() const { return crc ^ CRC32_NEGL; }
	
private:
	
	static const uint32_t CRC32_NEGL = 0xffffffffl;
	
	static const uint32_t table[256];
	uint32_t crc;
};

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_CRC32_HPP

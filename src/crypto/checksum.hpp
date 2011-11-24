
#ifndef INNOEXTRACT_CRYPTO_CHECKSUM_HPP
#define INNOEXTRACT_CRYPTO_CHECKSUM_HPP

#include <stdint.h>
#include <cstring>
#include <iosfwd>
#include <istream>

#include "util/endian.hpp"
#include "util/enum.hpp"
#include "util/types.hpp"

namespace crypto {

enum checksum_type {
	Adler32,
	CRC32,
	MD5,
	SHA1,
};

struct checksum {
	
	union {
		uint32_t adler32;
		uint32_t crc32;
		char md5[16];
		char sha1[20];
	};
	
	checksum_type type;
	
	bool operator==(const checksum & other) const;
	inline bool operator!=(const checksum & other) const { return !(*this == other); }
	
};

template <class Impl>
class checksum_base : public static_polymorphic<Impl> {
	
public:
	
	template <typename Endianness, class T>
	inline T process(T data) {
		char buf[sizeof(data)];
		T swapped = Endianness::byteswap_if_alien(data);
		std::memcpy(buf, &swapped, sizeof(swapped));
		this->impl().update(buf, sizeof(buf));
		return data;
	}
	
	/*!
	 * Load the data and process it.
	 * Data is processed as-is and then converted according to Endianness.
	 */
	template <typename Endianness, class T>
	inline T load(std::istream & is) {
		T result;
		char buf[sizeof(result)];
		is.read(buf, sizeof(buf));
		this->impl().update(buf, sizeof(buf));
		std::memcpy(&result, buf, sizeof(result));
		return Endianness::byteswap_if_alien(result);
	}
	
};

} // namespace crypto

NAMED_ENUM(crypto::checksum_type)

std::ostream & operator<<(std::ostream & os, const crypto::checksum & checksum);

#endif // INNOEXTRACT_CRYPTO_CHECKSUM_HPP

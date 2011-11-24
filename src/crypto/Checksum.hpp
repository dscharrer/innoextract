
#ifndef INNOEXTRACT_CRYPTO_CHECKSUM_HPP
#define INNOEXTRACT_CRYPTO_CHECKSUM_HPP

#include <stdint.h>
#include <cstring>
#include <iosfwd>
#include <istream>

#include "util/endian.hpp"
#include "util/enum.hpp"

struct Checksum {
	
	enum Type {
		Adler32,
		Crc32,
		MD5,
		Sha1,
	};
	
	union {
		uint32_t adler32;
		uint32_t crc32;
		char md5[16];
		char sha1[20];
	};
	
	Type type;
	
	bool operator==(const Checksum & other) const;
	inline bool operator!=(const Checksum & other) const { return !(*this == other); }
	
};

template <class Base>
class StaticPolymorphic {
	
protected:
	
	inline Base & impl() { return *static_cast<Base *>(this); }
	
	inline const Base & impl() const { return *static_cast<const Base *>(this); }
	
};

template <class Base>
class ChecksumBase : public StaticPolymorphic<Base> {
	
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

NAMED_ENUM(Checksum::Type)

std::ostream & operator<<(std::ostream & os, const Checksum & checksum);

#endif // INNOEXTRACT_CRYPTO_CHECKSUM_HPP

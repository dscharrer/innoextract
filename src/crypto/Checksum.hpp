
#ifndef INNOEXTRACT_CRYPTO_CHECKSUM_HPP
#define INNOEXTRACT_CRYPTO_CHECKSUM_HPP

#include <stdint.h>
#include <cstring>
#include "util/Enum.hpp"

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
	
	inline Base & impl() { return *reinterpret_cast<Base *>(this); }
	
	inline const Base & impl() const { return *reinterpret_cast<const Base *>(this); }
	
};

template <class Base>
class ChecksumBase : public StaticPolymorphic<Base> {
	
public:
	
	template <class T>
	inline T process(T data) {
		char buf[sizeof(data)];
		std::memcpy(&buf, &data, sizeof(data));
		this->impl().update(buf, sizeof(data));
		return data;
	}
	
};

NAMED_ENUM(Checksum::Type)

#endif // INNOEXTRACT_CRYPTO_CHECKSUM_HPP;

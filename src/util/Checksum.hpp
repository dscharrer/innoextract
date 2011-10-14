
#ifndef INNOEXTRACT_UTIL_CHECKSUM_HPP
#define INNOEXTRACT_UTIL_CHECKSUM_HPP

#include "util/Enum.hpp"
#include "util/Types.hpp"

struct Checksum {
	
	enum Type {
		Adler32,
		Crc32,
		MD5,
		Sha1,
	};
	
	union {
		u32 adler32;
		u32 crc32;
		char md5[16];
		char sha1[20];
	};
	
	Type type;
	
	void init(Type type);
	
	void process(const void * data, size_t size);
	
	template <class T>
	inline T process(T data) {
		process(&data, sizeof(data));
		return data;
	}
	
	void finalize();
	
	bool operator==(const Checksum & other) const;
	inline bool operator!=(const Checksum & other) const { return !(*this == other); }
	
	inline Checksum() { }
	inline Checksum(Type type) { init(type); }
	
};

NAMED_ENUM(Checksum::Type)

#endif // INNOEXTRACT_UTIL_CHECKSUM_HPP;

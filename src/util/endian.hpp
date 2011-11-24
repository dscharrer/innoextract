
#ifndef INNOEXTRACT_UTIL_ENDIAN_HPP
#define INNOEXTRACT_UTIL_ENDIAN_HPP

#include <cstdlib>

#include <boost/detail/endian.hpp>

inline uint8_t byteswap(uint8_t value) {
	return value;
}

inline int8_t byteswap(int8_t value) {
	return int8_t(byteswap(uint8_t(value)));
}

inline uint16_t byteswap(uint16_t value) {
#if defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_ushort(value);
#else
	return uint16_t((uint16_t(uint8_t(value)) << 8) | uint8_t(value >> 8));
#endif
}

inline int16_t byteswap(int16_t value) {
	return int16_t(byteswap(uint16_t(value)));
}

inline uint32_t byteswap(uint32_t value) {
#if defined(__GNUC__)
	return __builtin_bswap32(value);
#elif defined(_MSC_VER) && (_MSC_VER >= 1400 || (_MSC_VER >= 1300 && !defined(_DLL)))
	return _byteswap_ulong(value);
#else
	return (uint32_t(byteswap(uint16_t(value))) << 16) | byteswap(uint16_t(value >> 16));
#endif
}

inline int32_t byteswap(int32_t value) {
	return int32_t(byteswap(uint32_t(value)));
}

inline uint64_t byteswap(uint64_t value) {
#if defined(__GNUC__)
	return __builtin_bswap64(value);
#elif defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_uint64(value);
#else
	return (uint64_t(byteswap(uint32_t(value))) << 32) | byteswap(uint32_t(value >> 32));
#endif
}

inline int64_t byteswap(int64_t value) {
	return int64_t(byteswap(uint64_t(value)));
}

template <class T>
void byteswap(T * out, const T * in, size_t byteCount) {
	for(size_t i = 0; i < byteCount / sizeof(T); i++) {
		out[i] = byteswap(in[i]);
	}
}

template <bool Native>
struct endianness {
	
	static const size_t native = false;
	
	template <class T>
	static T byteswap_if_alien(T value) { return byteswap(value); }
	
	template <class T>
	static void byteswap_if_alien(const T * in, T * out, size_t byteCount) {
		byteswap(out, in, byteCount);
	}
	
};

template <>
struct endianness<true> {
	
	static const size_t native = true;
	
	template <class T>
	static T byteswap_if_alien(T value) { return value; }
	
	template <class T>
	static void byteswap_if_alien(const T * in, T * out, size_t byteCount) {
		if(in != out) {
			std::memcpy(out, in, byteCount);
		}
	}
	
};

struct little_endian : public endianness<BOOST_BYTE_ORDER == 1234> {
	static const size_t offset = 0;
};

struct big_endian : public endianness<BOOST_BYTE_ORDER == 4321> {
	static const size_t offset = 1;
};

#if defined(BOOST_LITTLE_ENDIAN)
typedef little_endian native_endian;
#elif defined(BOOST_BIG_ENDIAN)
typedef big_endian native_endian;
#else
#error "Unsupported host endianness."
#endif

#endif // INNOEXTRACT_UTIL_ENDIAN_HPP


#ifndef INNOEXTRACT_UTIL_ENDIAN_HPP
#define INNOEXTRACT_UTIL_ENDIAN_HPP

#include <boost/detail/endian.hpp>

inline uint8_t byteSwap(uint8_t value) {
	return value;
}

inline int8_t byteSwap(int8_t value) {
	return byteSwap(uint8_t(value));
}

inline uint16_t byteSwap(uint16_t value) {
#if defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_ushort(value);
#else
	return (uint16_t(uint8_t(value)) << 8) | uint8_t(value >> 8);
#endif
}

inline int16_t byteSwap(int16_t value) {
	return byteSwap(uint16_t(value));
}

inline uint32_t byteSwap(uint32_t value) {
#if defined(__GNUC__)
	return __builtin_bswap32(value);
#elif _MSC_VER >= 1400 || (_MSC_VER >= 1300 && !defined(_DLL))
	return _byteswap_ulong(value);
#else
	return (uint32_t(byteSwap(uint16_t(value))) << 16) | byteSwap(uint16_t(value >> 16));
#endif
}

inline int32_t byteSwap(int32_t value) {
	return byteSwap(uint32_t(value));
}

inline uint64_t byteSwap(uint64_t value) {
#if defined(__GNUC__)
	return __builtin_bswap64(value);
#elif defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_uint64(value);
#else
	return (uint64_t(byteSwap(uint32_t(value))) << 32) | byteSwap(uint32_t(value >> 32));
#endif
}

inline int64_t byteSwap(int64_t value) {
	return byteSwap(uint64_t(value));
}

template <class T>
void byteSwap(T * out, const T * in, size_t byteCount) {
	for(size_t i = 0; i < byteCount / sizeof(T); i++) {
		out[i] = byteSwap(in[i]);
	}
}

template <bool Native>
struct Endianness {
	
	static const size_t native = false;
	
	template <class T>
	static T byteSwapIfAlien(T value) { return byteSwap(value); }
	
	template <class T>
	static void byteSwapIfAlien(const T * in, T * out, size_t byteCount) {
		byteSwap(out, in, byteCount);
	}
	
};

template <>
struct Endianness<true> {
	
	static const size_t native = true;
	
	template <class T>
	static T byteSwapIfAlien(T value) { return value; }
	
	template <class T>
	static void byteSwapIfAlien(const T * in, T * out, size_t byteCount) {
		if(in != out) {
			memcpy(out, in, byteCount);
		}
	}
	
};

struct LittleEndian : public Endianness<BOOST_BYTE_ORDER == 1234> {
	
	static const size_t offset = 0;
	
};

struct BigEndian : public Endianness<BOOST_BYTE_ORDER == 4321> {
	
	static const size_t offset = 1;
	
};

#ifdef BOOST_LITTLE_ENDIAN
typedef LittleEndian NativeEndian;
#elif BOOLST_BIG_ENDIAN
typedef BigEndian NativeEndian;
#else
#error "Unsupported host endianness."
#endif

#endif // INNOEXTRACT_UTIL_ENDIAN_HPP

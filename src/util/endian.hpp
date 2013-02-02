/*
 * Copyright (C) 2011-2012 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef INNOEXTRACT_UTIL_ENDIAN_HPP
#define INNOEXTRACT_UTIL_ENDIAN_HPP

#include <cstdlib>

#include "configure.hpp"

#if INNOEXTRACT_HAVE_BSWAP_16 || INNOEXTRACT_HAVE_BSWAP_32 || INNOEXTRACT_HAVE_BSWAP_64
#include <byteswap.h>
#endif

#include <boost/cstdint.hpp>

inline boost::uint8_t byteswap(boost::uint8_t value) {
	return value;
}

inline boost::int8_t byteswap(boost::int8_t value) {
	return boost::int8_t(byteswap(boost::uint8_t(value)));
}

inline boost::uint16_t byteswap(boost::uint16_t value) {
#if defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_ushort(value);
#elif INNOEXTRACT_HAVE_BSWAP_16
	return bswap_16(value);
#else
	return boost::uint16_t((boost::uint16_t(boost::uint8_t(value)) << 8) | boost::uint8_t(value >> 8));
#endif
}

inline boost::int16_t byteswap(boost::int16_t value) {
	return boost::int16_t(byteswap(boost::uint16_t(value)));
}

inline boost::uint32_t byteswap(boost::uint32_t value) {
#if defined(__GNUC__) && !defined(__PATHCC__) \
    && __GNUC__ >= 4 && (__GNUC__ > 4 || __GNUC_MINOR__ >= 3)
	return __builtin_bswap32(value);
#elif defined(_MSC_VER) && (_MSC_VER >= 1400 || (_MSC_VER >= 1300 && !defined(_DLL)))
	return _byteswap_ulong(value);
#elif INNOEXTRACT_HAVE_BSWAP_32
	return bswap_32(value);
#else
	return (boost::uint32_t(byteswap(boost::uint16_t(value))) << 16) | byteswap(boost::uint16_t(value >> 16));
#endif
}

inline boost::int32_t byteswap(boost::int32_t value) {
	return boost::int32_t(byteswap(boost::uint32_t(value)));
}

inline boost::uint64_t byteswap(boost::uint64_t value) {
#if defined(__GNUC__) && !defined(__PATHCC__) \
    && __GNUC__ >= 4 && (__GNUC__ > 4 || __GNUC_MINOR__ >= 3)
	return __builtin_bswap64(value);
#elif defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_uint64(value);
#elif INNOEXTRACT_HAVE_BSWAP_64
	return bswap_64(value);
#else
	return (boost::uint64_t(byteswap(boost::uint32_t(value))) << 32) | byteswap(boost::uint32_t(value >> 32));
#endif
}

inline boost::int64_t byteswap(boost::int64_t value) {
	return boost::int64_t(byteswap(boost::uint64_t(value)));
}

template <class T>
void byteswap(T * out, const T * in, size_t byte_count) {
	for(size_t i = 0; i < byte_count / sizeof(T); i++) {
		out[i] = byteswap(in[i]);
	}
}

template <bool Native>
struct endianness {
	
	static const size_t native = false;
	
	template <class T>
	static T byteswap_if_alien(T value) { return byteswap(value); }
	
	template <class T>
	static void byteswap_if_alien(const T * in, T * out, size_t byte_count) {
		byteswap(out, in, byte_count);
	}
	
};

template <>
struct endianness<true> {
	
	static const size_t native = true;
	
	template <class T>
	static T byteswap_if_alien(T value) { return value; }
	
	template <class T>
	static void byteswap_if_alien(const T * in, T * out, size_t byte_count) {
		if(in != out) {
			std::memcpy(out, in, byte_count);
		}
	}
	
};

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN    4321
#endif

#if INNOEXTRACT_IS_BIG_ENDIAN
#define INNOEXTRACT_ENDIANNESS    BIG_ENDIAN
#else
#define INNOEXTRACT_ENDIANNESS    LITTLE_ENDIAN
#endif


struct little_endian : public endianness<INNOEXTRACT_ENDIANNESS == LITTLE_ENDIAN> {
	static const size_t offset = 0;
};

struct big_endian : public endianness<INNOEXTRACT_ENDIANNESS == BIG_ENDIAN> {
	static const size_t offset = 1;
};

#if INNOEXTRACT_ENDIANNESS == LITTLE_ENDIAN
typedef little_endian native_endian;
#elif INNOEXTRACT_ENDIANNESS == BIG_ENDIAN
typedef big_endian native_endian;
#else
#error "Unsupported host endianness."
#endif

#endif // INNOEXTRACT_UTIL_ENDIAN_HPP

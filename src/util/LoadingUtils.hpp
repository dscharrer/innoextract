
#ifndef INNOEXTRACT_UTIL_LOADINGUTILS_HPP
#define INNOEXTRACT_UTIL_LOADINGUTILS_HPP

#include <stdint.h>
#include <iostream>
#include <string>
#include <limits>
#include <boost/detail/endian.hpp>

inline uint8_t fromLittleEndian(uint8_t value) { return value; }
inline int8_t fromLittleEndian(int8_t value) { return value; }

#ifdef BOOST_LITTLE_ENDIAN

inline uint16_t fromLittleEndian(uint16_t value) { return value; }
inline uint32_t fromLittleEndian(uint32_t value) { return value; }
inline uint64_t fromLittleEndian(uint64_t value) { return value; }
inline int16_t fromLittleEndian(int16_t value) { return value; }
inline int32_t fromLittleEndian(int32_t value) { return value; }
inline int64_t fromLittleEndian(int64_t value) { return value; }

#else 

// TODO implement!
#error "Host endianness not supported!"

#endif

struct BinaryString {
	
	std::string & data;
	
	inline BinaryString(std::string & target) : data(target) { }
	
	static void loadInto(std::istream & is, std::string & target);
	
};
inline std::istream & operator>>(std::istream & is, const BinaryString & str) {
	BinaryString::loadInto(is, str.data);
	return is;
}

void toUtf8(const std::string & from, std::string & to, uint32_t codepage = 1252);

struct EncodedString {
	
	std::string & data;
	uint32_t codepage;
	
	inline EncodedString(std::string & target, uint32_t _codepage) : data(target), codepage(_codepage) { }
	
	static void loadInto(std::istream & is, std::string & target, uint32_t codepage);
	
};

inline std::istream & operator>>(std::istream & is, const EncodedString & str) {
	EncodedString::loadInto(is, str.data, str.codepage);
	return is;
}

struct AnsiString : EncodedString {
	
	inline AnsiString(std::string & target) : EncodedString(target, 1252) { }
	
};

template <class T>
inline T load(std::istream & is) {
	T value;
	is.read(reinterpret_cast<char *>(&value), sizeof(value));
	return value;
}

template <class T>
inline T loadNumber(std::istream & is) {
	return fromLittleEndian(load<T>(is));
}

template <class Base, size_t Bits, bool Signed = std::numeric_limits<Base>::is_signed>
struct compatible_integer { typedef void type; };
template <class Base>
struct compatible_integer<Base, 8, false> { typedef uint8_t type; };
template <class Base>
struct compatible_integer<Base, 8, true> { typedef int8_t type; };
template <class Base>
struct compatible_integer<Base, 16, false> { typedef uint16_t type; };
template <class Base>
struct compatible_integer<Base, 16, true> { typedef int16_t type; };
template <class Base>
struct compatible_integer<Base, 32, false> { typedef uint32_t type; };
template <class Base>
struct compatible_integer<Base, 32, true> { typedef int32_t type; };
template <class Base>
struct compatible_integer<Base, 64, false> { typedef uint64_t type; };
template <class Base>
struct compatible_integer<Base, 64, true> { typedef int64_t type; };

template <class T>
T loadNumber(std::istream & is, size_t bits) {
	if(bits == 8) {
		return loadNumber<typename compatible_integer<T, 8>::type>(is);
	} else if(bits == 16) {
		return loadNumber<typename compatible_integer<T, 16>::type>(is);
	} else if(bits == 32) {
		return loadNumber<typename compatible_integer<T, 32>::type>(is);
	} else {
		return loadNumber<typename compatible_integer<T, 64>::type>(is);
	}
}

#endif // INNOEXTRACT_UTIL_LOADINGUTILS_HPP


#ifndef INNOEXTRACT_UTIL_LOADINGUTILS_HPP
#define INNOEXTRACT_UTIL_LOADINGUTILS_HPP

#include <stdint.h>
#include <cstring>
#include <iostream>
#include <string>
#include <limits>

#include <boost/integer/static_min_max.hpp>
#include <boost/integer.hpp>

#include "util/Endian.hpp"

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
	
	inline EncodedString(std::string & target, uint32_t _codepage)
		: data(target), codepage(_codepage) { }
	
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
	char buffer[sizeof(value)];
	is.read(buffer, sizeof(buffer));
	std::memcpy(&value, buffer, sizeof(value));
	return value;
}

template <class T>
inline T loadNumber(std::istream & is) {
	return LittleEndian::byteSwapIfAlien(load<T>(is));
}

template <class Base, size_t Bits,
          bool Signed = std::numeric_limits<Base>::is_signed>
struct compatible_integer {
	typedef void type;
};

template <class Base, size_t Bits>
struct compatible_integer<Base, Bits, false> {
	typedef typename boost::uint_t<
		boost::static_unsigned_min<Bits, sizeof(Base) * 8>::value
	>::exact type;
};

template <class Base, size_t Bits>
struct compatible_integer<Base, Bits, true> {
	typedef typename boost::int_t<
		boost::static_unsigned_min<Bits, sizeof(Base) * 8>::value
	>::exact type;
};

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

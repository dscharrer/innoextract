
#ifndef INNOEXTRACT_LOADINGUTILS_HPP
#define INNOEXTRACT_LOADINGUTILS_HPP

#include <iostream>
#include <string>
#include <limits>
#include <boost/detail/endian.hpp>
#include "Types.hpp"

inline u8 fromLittleEndian(u8 value) { return value; }
inline s8 fromLittleEndian(s8 value) { return value; }

#ifdef BOOST_LITTLE_ENDIAN

inline u16 fromLittleEndian(u16 value) { return value; }
inline u32 fromLittleEndian(u32 value) { return value; }
inline u64 fromLittleEndian(u64 value) { return value; }
inline s16 fromLittleEndian(s16 value) { return value; }
inline s32 fromLittleEndian(s32 value) { return value; }
inline s64 fromLittleEndian(s64 value) { return value; }

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

struct AnsiString {
	
	std::string & data;
	
	inline AnsiString(std::string & target) : data(target) { }
	
	static void loadInto(std::istream & is, std::string & target);
	
};

inline std::istream & operator>>(std::istream & is, const AnsiString & str) {
	AnsiString::loadInto(is, str.data);
	return is;
}

struct WideString {
	
	std::string & data;
	bool wide;
	
	inline WideString(std::string & target, bool _wide /*= true*/) : data(target), wide(_wide) { }
	
	static void loadInto(std::istream & is, std::string & target);
	
};

inline std::istream & operator>>(std::istream & is, const WideString & str) {
	str.wide ? WideString::loadInto(is, str.data) : AnsiString::loadInto(is, str.data);
	return is;
}

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
struct compatible_integer<Base, 8, false> { typedef u8 type; };
template <class Base>
struct compatible_integer<Base, 8, true> { typedef s8 type; };
template <class Base>
struct compatible_integer<Base, 16, false> { typedef u16 type; };
template <class Base>
struct compatible_integer<Base, 16, true> { typedef s16 type; };
template <class Base>
struct compatible_integer<Base, 32, false> { typedef u32 type; };
template <class Base>
struct compatible_integer<Base, 32, true> { typedef s32 type; };
template <class Base>
struct compatible_integer<Base, 64, false> { typedef u64 type; };
template <class Base>
struct compatible_integer<Base, 64, true> { typedef s64 type; };

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

#endif // INNOEXTRACT_LOADINGUTILS_HPP

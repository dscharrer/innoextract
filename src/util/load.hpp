
#ifndef INNOEXTRACT_UTIL_LOADINGUTILS_HPP
#define INNOEXTRACT_UTIL_LOADINGUTILS_HPP

#include <stdint.h>
#include <cstring>
#include <iostream>
#include <string>

#include "util/endian.hpp"
#include "util/types.hpp"

struct binary_string {
	
	std::string & data;
	
	inline binary_string(std::string & target) : data(target) { }
	
	static void load(std::istream & is, std::string & target);
	
};

inline std::istream & operator>>(std::istream & is, const binary_string & str) {
	binary_string::load(is, str.data);
	return is;
}

void to_utf8(const std::string & from, std::string & to, uint32_t codepage = 1252);

struct encoded_string {
	
	std::string & data;
	uint32_t codepage;
	
	inline encoded_string(std::string & target, uint32_t _codepage)
		: data(target), codepage(_codepage) { }
	
	static void load(std::istream & is, std::string & target, uint32_t codepage);
	
};

inline std::istream & operator>>(std::istream & is, const encoded_string & str) {
	encoded_string::load(is, str.data, str.codepage);
	return is;
}

struct ansi_string : encoded_string {
	
	inline ansi_string(std::string & target) : encoded_string(target, 1252) { }
	
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
inline T load_number(std::istream & is) {
	return little_endian::byteswap_if_alien(load<T>(is));
}

template <class T>
T load_number(std::istream & is, size_t bits) {
	if(bits == 8) {
		return load_number<typename compatible_integer<T, 8>::type>(is);
	} else if(bits == 16) {
		return load_number<typename compatible_integer<T, 16>::type>(is);
	} else if(bits == 32) {
		return load_number<typename compatible_integer<T, 32>::type>(is);
	} else {
		return load_number<typename compatible_integer<T, 64>::type>(is);
	}
}

#endif // INNOEXTRACT_UTIL_LOADINGUTILS_HPP

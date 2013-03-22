/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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

/*!
 * Utility function to load stored data while properly handling encodings and endianness.
 */
#ifndef INNOEXTRACT_UTIL_LOAD_HPP
#define INNOEXTRACT_UTIL_LOAD_HPP

#include <cstring>
#include <istream>
#include <string>

#include <boost/cstdint.hpp>

#include "util/endian.hpp"
#include "util/types.hpp"
#include "util/util.hpp"

namespace util {

typedef boost::uint32_t codepage_id;

/*!
 * Convert a string to UTF-8 from a specified encoding.
 * \param from     The input string to convert.
 * \param to       The output for the converted string.
 * \param codepage The Windows codepage number for the input string encoding.
 *
 * \note This function is not thread-safe.
 */
void to_utf8(const std::string & from, std::string & to, codepage_id codepage = 1252);

/*!
 * Wrapper to load a length-prefixed string from an input stream into a std::string.
 * The string length is stored as 32-bit integer.
 *
 * Usage: <code>is >> binary_string(str)</code>
 *
 * Use \ref encoded_string to also convert the string to UTF-8.
 */
struct binary_string {
	
	std::string & data;
	
	/*!
	 * \param target The std::string object to receive the loaded string.
	 */
	explicit binary_string(std::string & target) : data(target) { }
	
	//! Load a length-prefixed string
	static void load(std::istream & is, std::string & target);
	
	static void skip(std::istream & is);
	
	//! Load a length-prefixed string
	static std::string load(std::istream & is) {
		std::string target;
		load(is, target);
		return target;
	}
	
};
inline std::istream & operator>>(std::istream & is, const binary_string & str) {
	binary_string::load(is, str.data);
	return is;
}

/*!
 * Wrapper to load a length-prefixed string with a specified encoding from an input stream
 * into a UTF-8 encoded std::string.
 * The string length is stored as 32-bit integer.
 *
 * Usage: <code>is >> encoded_string(str, codepage)</code>
 *
 * You can also use the \ref ansi_string convenience wrapper for Windows-1252 strings.
 *
 * \note This wrapper is not thread-safe.
 */
struct encoded_string {
	
	std::string & data;
	codepage_id codepage;
	
	/*!
	 * \param target   The std::string object to receive the loaded UTF-8 string.
	 * \param codepage The Windows codepage for the encoding of the stored string.
	 */
	encoded_string(std::string & target, codepage_id codepage)
		: data(target), codepage(codepage) { }
	
	/*!
	 * Load and convert a length-prefixed string
	 *
	 * \note This function is not thread-safe.
	 */
	static void load(std::istream & is, std::string & target, codepage_id codepage);
	
	/*!
	 * Load and convert a length-prefixed string
	 *
	 * \note This function is not thread-safe.
	 */
	static std::string load(std::istream & is, codepage_id codepage) {
		std::string target;
		load(is, target, codepage);
		return target;
	}
	
};
inline std::istream & operator>>(std::istream & is, const encoded_string & str) {
	encoded_string::load(is, str.data, str.codepage);
	return is;
}

/*!
 * Convenience specialization of \ref encoded_string for loading Windows-1252 strings
 *
 * \note This function is not thread-safe.
 */
struct ansi_string : encoded_string {
	
	explicit ansi_string(std::string & target) : encoded_string(target, 1252) { }
	
};

//! Load a value of type T that is stored with a specific endianness.
template <class T, class Endianness>
T load(std::istream & is) {
	char buffer[sizeof(T)];
	is.read(buffer, std::streamsize(sizeof(buffer)));
	return Endianness::template load<T>(buffer);
}
//! Load a value of type T that is stored as little endian.
template <class T>
T load(std::istream & is) { return load<T, little_endian>(is); }

//! Load a bool value
inline bool load_bool(std::istream & is) {
	return !!load<boost::uint8_t>(is);
}

/*!
 * Load a value of type T that is stored with a specific endianness.
 * \param is   Input stream to load from.
 * \param bits The number of bits used to store the number.
 */
template <class T, class Endianness>
T load(std::istream & is, size_t bits) {
	if(bits == 8) {
		return load<typename compatible_integer<T, 8>::type, Endianness>(is);
	} else if(bits == 16) {
		return load<typename compatible_integer<T, 16>::type, Endianness>(is);
	} else if(bits == 32) {
		return load<typename compatible_integer<T, 32>::type, Endianness>(is);
	} else {
		return load<typename compatible_integer<T, 64>::type, Endianness>(is);
	}
}
/*!
 * Load a value of type T that is stored as little endian.
 * \param is   Input stream to load from.
 * \param bits The number of bits used to store the number.
 */
template <class T>
T load(std::istream & is, size_t bits) { return load<T, little_endian>(is, bits); }

/*!
 * Discard a number of bytes from a non-seekable input stream or stream-like object
 * \param is    The stream to "seek"
 * \param bytes Number of bytes to skip ahead
 */
template <class T>
void discard(T & is, boost::uint64_t bytes) {
	char buf[1024];
	while(bytes) {
		std::streamsize n = std::streamsize(std::min<boost::uint64_t>(bytes, ARRAY_SIZE(buf)));
		is.read(buf, n);
		bytes -= boost::uint64_t(n);
	}
}

/*!
 * Get the number represented by a specific range of bits of another number.
 * All other bis are masked and the requested bits are shifted to position 0.
 * \param number The number containing the desired bits.
 * \param first  Index of the first desired bit.
 * \param last   Index of the last desired bit (inclusive).
 */
template <typename T>
T get_bits(T number, int first, int last) {
	typedef typename uint_t<sizeof(T) * 8>::exact UT;
	UT data = UT(number);
	data = UT(data >> first), last -= first;
	UT mask = UT(((last + 1 == sizeof(T) * 8) ? UT(0) : UT(UT(1) << (last + 1))) - 1);
	return T(data & mask);
}

} // namespace util

#endif // INNOEXTRACT_UTIL_LOAD_HPP

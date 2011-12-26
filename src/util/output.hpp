/*
 * Copyright (C) 2011 Daniel Scharrer
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

#ifndef INNOEXTRACT_UTIL_OUTPUT_HPP
#define INNOEXTRACT_UTIL_OUTPUT_HPP

#include <stdint.h>
#include <iostream>
#include <string>

#include "util/console.hpp"
#include "util/util.hpp"

struct quoted {
	
	const std::string & str;
	
	explicit quoted(const std::string & _str) : str(_str) { }
	
};

inline std::ostream & operator<<(std::ostream & os, const quoted & q) {
	color::shell_command prev = color::current;
	os << '"' << color::green;
	for(std::string::const_iterator i = q.str.begin(); i != q.str.end(); ++i) {
		unsigned char c = (unsigned char)*i;
		if(c < ' ' && c != '\t' && c != '\r' && c != '\n') {
			std::ios_base::fmtflags old = os.flags();
			os << color::red << '<' << std::hex << std::setfill('0') << std::setw(2)
			   << int(c) << '>' << color::green;
			os.setf(old, std::ios_base::basefield);
		} else {
			os << *i;
		}
	}
	return os << prev << '"';
}

struct if_not_empty {
	
	const std::string & name;
	const std::string & value;
	
	if_not_empty(const std::string & _name, const std::string & _value)
		: name(_name), value(_value) { }
	
};

inline std::ostream & operator<<(std::ostream & os, const if_not_empty & s) {
	if(s.value.length() > 100) {
		color::shell_command prev = color::current;
		return os << s.name << ": " << color::white << s.value.length() << prev
		          << " bytes" << '\n';
	} else if(!s.value.empty()) {
		return os << s.name << ": " << quoted(s.value) << '\n';
	} else {
		return os;
	}
}

namespace detail {

template <class T>
struct if_not {
	
	const std::string & name;
	const T value;
	const T excluded;
	
	if_not(const std::string & _name, T _value, T _excluded)
		: name(_name), value(_value), excluded(_excluded) { }
	
};

template <class T>
inline std::ostream & operator<<(std::ostream & os, const if_not<T> & s) {
	if(s.value != s.excluded) {
		color::shell_command prev = color::current;
		return os << s.name << ": " << color::cyan << s.value << prev << '\n';
	} else {
		return os;
	}
}

}


template <class T>
detail::if_not<T> if_not_equal(const std::string & name, T value, T excluded) {
	return detail::if_not<T>(name, value, excluded);
}

template <class T>
detail::if_not<T> if_not_zero(const std::string & name, T value) {
	return detail::if_not<T>(name, value, T(0));
}

namespace detail {

template <class T>
struct print_hex {
	
	T value;
	
	explicit print_hex(T data) : value(data) { }
	
	bool operator==(const print_hex & o) const { return value == o.value; }
	bool operator!=(const print_hex & o) const { return value != o.value; }
	
};

template <class T>
std::ostream & operator<<(std::ostream & os, const print_hex<T> & s) {
	
	std::ios_base::fmtflags old = os.flags();
	
	os << "0x" << std::hex << s.value;
	
	os.setf(old, std::ios_base::basefield);
	return os;
}

}

template <class T>
detail::print_hex<T> print_hex(T value) {
	return detail::print_hex<T>(value);
}

const char * const byte_size_units[5] = {
	"B",
	"KiB",
	"MiB",
	"GiB",
	"TiB"
};

namespace detail {

template <class T>
struct print_bytes {
	
	T value;
	
	explicit print_bytes(T data) : value(data) { }
	
	bool operator==(const print_bytes & o) const { return value == o.value; }
	bool operator!=(const print_bytes & o) const { return value != o.value; }
	
};

template <class T>
inline std::ostream & operator<<(std::ostream & os, const print_bytes<T> & s) {
	
	std::streamsize precision = os.precision();
	
	size_t frac = size_t(1024 * (s.value - T(uint64_t(s.value))));
	uint64_t whole = uint64_t(s.value);
	
	size_t i = 0;
	
	while(whole > 1024 && i < ARRAY_SIZE(byte_size_units) - 1) {
		
		frac = whole % 1024, whole /= 1024;
		
		i++;
	}
	
	float num = float(whole) + (float(frac) / 1024.f);
	
	os << std::setprecision(3) << num << ' ' << byte_size_units[i];
	
	os.precision(precision);
	return os;
}

}

template <class T>
detail::print_bytes<T> print_bytes(T value) {
	return detail::print_bytes<T>(value);
}

#endif // INNOEXTRACT_UTIL_OUTPUT_HPP

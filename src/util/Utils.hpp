
#ifndef INNOEXTRACT_UTIL_UTILS_HPP
#define INNOEXTRACT_UTIL_UTILS_HPP

#include <iostream>
#include <string>

#include "util/Output.hpp"

// TODO remove
template <class T>
inline std::istream & read(std::istream & ifs, T & data) {
	return ifs.read(reinterpret_cast<char *>(&data), sizeof(T));
}
/**
 * Load an std::string from a const char * that may not be null-terminated.
 */
std::string safestring(const char * data, size_t maxLength);

template <size_t N>
std::string safestring(const char (&data)[N]) {
	return safestring(data, N);
}

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(*(array)))

struct Quoted {
	
	const std::string & str;
	
	Quoted(const std::string & _str) : str(_str) { }
	
};
inline std::ostream & operator<<(std::ostream & os, const Quoted & q) {
	color::shell_command prev = color::current;
	os << '"' << color::green;
	for(std::string::const_iterator i = q.str.begin(); i != q.str.end(); ++i) {
		unsigned char c = (unsigned char)*i;
		if(c < ' ' && c != '\t' && c != '\r' && c != '\n') {
			std::ios_base::fmtflags old = os.flags();
			os << color::red << '<' << std::hex << std::setfill('0') << std::setw(2) << int(c) << '>' << color::green;
			os.setf(old, std::ios_base::basefield);
		} else {
			os << *i;
		}
	}
	return os << prev << '"';
}

struct IfNotEmpty {
	
	const std::string & name;
	const std::string & value;
	
	IfNotEmpty(const std::string & _name, const std::string & _value) : name(_name), value(_value) { }
	
};
inline std::ostream & operator<<(std::ostream & os, const IfNotEmpty & s) {
	if(s.value.length() > 100) {
		color::shell_command prev = color::current;
		return os << s.name << ": " << color::white << s.value.length() << prev << " bytes" << std::endl;
	} else if(!s.value.empty()) {
		return os << s.name << ": " << Quoted(s.value) << std::endl;
	} else {
		return os;
	}
}

template <class T>
struct _IfNot {
	
	const std::string & name;
	const T value;
	const T excluded;
	
	_IfNot(const std::string & _name, T _value, T _excluded) : name(_name), value(_value), excluded(_excluded) { }
	
};
template <class T>
inline std::ostream & operator<<(std::ostream & os, const _IfNot<T> & s) {
	if(s.value != s.excluded) {
		color::shell_command prev = color::current;
		return os << s.name << ": " << color::cyan << s.value << prev << std::endl;
	} else {
		return os;
	}
}
template <class T>
_IfNot<T> IfNot(const std::string & name, T value, T excluded) {
	return _IfNot<T>(name, value, excluded);
}
template <class T>
_IfNot<T> IfNotZero(const std::string & name, T value) {
	return _IfNot<T>(name, value, T(0));
}

template <class A, class B>
inline A ceildiv(A num, B denom) {
	return A((num + (denom - 1)) / denom);
}

template <class T>
struct _PrintHex {
	
	T value;
	
	_PrintHex(T data) : value(data) { }
	
	bool operator==(const _PrintHex & o) const { return value == o.value; }
	bool operator!=(const _PrintHex & o) const { return value != o.value; }
	
};
template <class T>
inline std::ostream & operator<<(std::ostream & os, const _PrintHex<T> & s) {
	
	std::ios_base::fmtflags old = os.flags();
	
	os << "0x" << std::hex << s.value;
	
	os.setf(old, std::ios_base::basefield);
	return os;
}
template <class T>
_PrintHex<T> PrintHex(T value) {
	return _PrintHex<T>(value);
}

const char * const byteSizeUnits[5] = {
	"B",
	"KiB",
	"MiB",
	"GiB",
	"TiB"
};

template <class T>
struct _PrintBytes {
	
	T value;
	
	_PrintBytes(T data) : value(data) { }
	
	bool operator==(const _PrintBytes & o) const { return value == o.value; }
	bool operator!=(const _PrintBytes & o) const { return value != o.value; }
	
};
template <class T>
inline std::ostream & operator<<(std::ostream & os, const _PrintBytes<T> & s) {
	
	int precision = os.precision();
	
	size_t frac = 0;
	size_t whole = s.value;
	
	size_t i = 0;
	
	while((whole & ~0x3ff) && i < ARRAY_SIZE(byteSizeUnits) - 1) {
		
		frac = (whole & 0x3ff), whole >>= 10;
		
		i++;
	}
	
	float num = whole + (frac / 1024.f);
	
	os << std::setprecision(3) << num << ' ' << byteSizeUnits[i];
	
	os.precision(precision);
	return os;
}
template <class T>
_PrintBytes<T> PrintBytes(T value) {
	return _PrintBytes<T>(value);
}

#endif // INNOEXTRACT_UTIL_UTILS_HPP

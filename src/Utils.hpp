
#ifndef INNOEXTRACT_UTILS_HPP
#define INNOEXTRACT_UTILS_HPP

#include <iostream>
#include <string>

#include "Output.hpp"

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
	return os << '"' << color::green << q.str << prev << '"';
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

#endif // INNOEXTRACT_UTILS_HPP

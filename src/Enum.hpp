
#ifndef INNOEXTRACT_ENUM_HPP
#define INNOEXTRACT_ENUM_HPP

#include <iostream>
#include <boost/static_assert.hpp>
#include "Utils.hpp"
#include "Flags.hpp"
#include "Output.hpp"

template <class Enum>
struct EnumNames {
	
	const size_t count;
	
	const char * name;
	
	const char * names[0];
	
};

#define NAMED_ENUM(Enum) \
	template <> struct EnumNames<Enum> { \
		static const char * name; \
		static const char * names[]; \
		static const size_t count; \
	}; \
	std::ostream & operator<<(std::ostream & os, Enum value);

#define ENUM_NAMES(Enum, Name, ...) \
	const char * EnumNames<Enum>::name = (Name); \
	const char * EnumNames<Enum>::names[] = { __VA_ARGS__ }; \
	const size_t EnumNames<Enum>::count = ARRAY_SIZE(EnumNames<Enum>::names); \
	std::ostream & operator<<(std::ostream & os, Enum value) { \
		if(value < EnumNames<Enum>::count) { \
			return os << EnumNames<Enum>::names[value]; \
		} else { \
			return os << "(unknown:" << int(value) << ')'; \
		} \
	}

template <class Enum>
std::ostream & operator<<(std::ostream & os, Flags<Enum> flags) {
	color::shell_command prev = color::current;
	if(flags) {
		bool first = true;
		for(size_t i = 0; i < Flags<Enum>::bits; i++) {
			if(flags & Enum(i)) {
				if(first) {
					first = false;
				} else {
					os << color::dim_white << ", " << prev;
				}
				os << Enum(i);
			}
		}
		return os;
	} else {
		return os << color::dim_white << "(none)" << prev;
	}
}

#endif // INNOEXTRACT_ENUM_HPP

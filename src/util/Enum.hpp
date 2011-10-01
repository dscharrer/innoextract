
#ifndef INNOEXTRACT_UTIL_ENUM_HPP
#define INNOEXTRACT_UTIL_ENUM_HPP

#include <iostream>
#include <boost/static_assert.hpp>

#include "util/Flags.hpp"
#include "util/Output.hpp"
#include "util/Utils.hpp"

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

#endif // INNOEXTRACT_UTIL_ENUM_HPP

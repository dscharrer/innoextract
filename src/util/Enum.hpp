
#ifndef INNOEXTRACT_UTIL_ENUM_HPP
#define INNOEXTRACT_UTIL_ENUM_HPP

#include <ostream>
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
	template <> struct EnumNames<GetEnum<Enum>::type> { \
		static const char * name; \
		static const char * names[]; \
		static const size_t count; \
	}; \
	std::ostream & operator<<(std::ostream & os, GetEnum<Enum>::type value);

#define ENUM_NAMES(Enum, Name, ...) \
	const char * EnumNames<GetEnum<Enum>::type>::name = (Name); \
	const char * EnumNames<GetEnum<Enum>::type>::names[] = { __VA_ARGS__ }; \
	const size_t EnumNames<GetEnum<Enum>::type>::count = ARRAY_SIZE(EnumNames<GetEnum<Enum>::type>::names); \
	std::ostream & operator<<(std::ostream & os, GetEnum<Enum>::type value) { \
		if(value < EnumNames<GetEnum<Enum>::type>::count) { \
			return os << EnumNames<GetEnum<Enum>::type>::names[value]; \
		} else { \
			return os << "(unknown:" << int(value) << ')'; \
		} \
	}

#endif // INNOEXTRACT_UTIL_ENUM_HPP


#ifndef INNOEXTRACT_UTIL_ENUM_HPP
#define INNOEXTRACT_UTIL_ENUM_HPP


#include <ostream>
#include <boost/utility/enable_if.hpp>

#include "util/Flags.hpp"
#include "util/Output.hpp"
#include "util/Utils.hpp"

template <class Enum>
struct EnumNames {
	
	const size_t count;
	
	const char * name;
	
	const char * names[];
	
};

#define NAMED_ENUM(Enum) \
	template <> struct EnumNames<GetEnum<Enum>::type> { \
		enum { named = 1 }; \
		static const char * name; \
		static const char * names[]; \
		static const size_t count; \
	};

#define ENUM_NAMES(Enum, Name, ...) \
	const char * EnumNames<GetEnum<Enum>::type>::name = (Name); \
	const char * EnumNames<GetEnum<Enum>::type>::names[] = { __VA_ARGS__ }; \
	const size_t EnumNames<GetEnum<Enum>::type>::count \
	 = ARRAY_SIZE(EnumNames<GetEnum<Enum>::type>::names);

template <class Enum>
typename boost::enable_if_c<EnumNames<Enum>::named, std::ostream &>::type
operator<<(std::ostream & os, Enum value) {
	if(value < EnumNames<Enum>::count) {
		return os << EnumNames<Enum>::names[value];
	} else {
		return os << "(unknown:" << int(value) << ')';
	}
}

#endif // INNOEXTRACT_UTIL_ENUM_HPP

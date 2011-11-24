
#ifndef INNOEXTRACT_UTIL_ENUM_HPP
#define INNOEXTRACT_UTIL_ENUM_HPP

#include <ostream>
#include <boost/utility/enable_if.hpp>

#include "util/console.hpp"
#include "util/flags.hpp"
#include "util/util.hpp"

template <class Enum>
struct get_enum {
	typedef Enum type;
};
template <class Enum>
struct get_enum< flags<Enum> > {
	typedef Enum type;
};

template <class Enum>
struct enum_names {
	
	const size_t count;
	
	const char * name;
	
	const char * names[];
	
};

#define NAMED_ENUM(Enum) \
	template <> struct enum_names<get_enum<Enum>::type> { \
		enum { named = 1 }; \
		static const char * name; \
		static const char * names[]; \
		static const size_t count; \
	};

#define ENUM_NAMES(Enum, Name, ...) \
	const char * enum_names<get_enum<Enum>::type>::name = (Name); \
	const char * enum_names<get_enum<Enum>::type>::names[] = { __VA_ARGS__ }; \
	const size_t enum_names<get_enum<Enum>::type>::count \
	 = ARRAY_SIZE(enum_names<get_enum<Enum>::type>::names);

template <class Enum>
typename boost::enable_if_c<enum_names<Enum>::named, std::ostream &>::type
operator<<(std::ostream & os, Enum value) {
	if(value < enum_names<Enum>::count) {
		return os << enum_names<Enum>::names[value];
	} else {
		return os << "(unknown:" << int(value) << ')';
	}
}

template <class Enum>
std::ostream & operator<<(std::ostream & os, flags<Enum> _flags) {
	color::shell_command prev = color::current;
	if(_flags) {
		bool first = true;
		for(size_t i = 0; i < flags<Enum>::bits; i++) {
			if(_flags & Enum(i)) {
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

#endif // INNOEXTRACT_UTIL_ENUM_HPP

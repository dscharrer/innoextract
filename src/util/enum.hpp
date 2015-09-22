/*
 * Copyright (C) 2011-2014 Daniel Scharrer
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
 * \file
 *
 * Utilities to associate strings with enum values.
 */
#ifndef INNOEXTRACT_UTIL_ENUM_HPP
#define INNOEXTRACT_UTIL_ENUM_HPP

#include <stddef.h>
#include <ostream>

#include <boost/range/size.hpp>
#include <boost/utility/enable_if.hpp>

#include "util/console.hpp"
#include "util/flags.hpp"

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
	
	const char * names[1];
	
};

#define NAMED_ENUM(Enum) \
	template <> struct enum_names<get_enum<Enum>::type> { \
		enum { named = 1 }; \
		static const char * name; \
		static const char * names[]; \
		static const size_t count; \
	};
	
#define NAMED_FLAGS(Flags) \
	FLAGS_OVERLOADS(Flags) \
	NAMED_ENUM(Flags)

#define NAMES(Enum, Name, ...) \
	const char * enum_names<get_enum<Enum>::type>::name = (Name); \
	const char * enum_names<get_enum<Enum>::type>::names[] = { __VA_ARGS__ }; \
	const size_t enum_names<get_enum<Enum>::type>::count \
	 = size_t(boost::size(enum_names<get_enum<Enum>::type>::names));

#define USE_ENUM_NAMES(Enum) \
	(void)enum_names<get_enum<Enum>::type>::count; \
	(void)enum_names<get_enum<Enum>::type>::name; \
	(void)enum_names<get_enum<Enum>::type>::names;

#define USE_FLAG_NAMES(Flags) \
	USE_FLAGS_OVERLOADS(Flags) \
	USE_ENUM_NAMES(Flags)

template <class Enum>
typename boost::enable_if_c<enum_names<Enum>::named, std::ostream &>::type
operator<<(std::ostream & os, Enum value) {
	if(value >= Enum(0)) {
		size_t i = size_t(value);
		if(i < enum_names<Enum>::count) {
			return os << enum_names<Enum>::names[value];
		}
	}
	return os << "(unknown:" << int(value) << ')';
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

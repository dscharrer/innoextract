/*
 * Copyright (C) 2011-2019 Daniel Scharrer
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
 * Typesafe flags.
 */
#ifndef INNOEXTRACT_UTIL_FLAGS_HPP
#define INNOEXTRACT_UTIL_FLAGS_HPP

#include <stddef.h>
#include <bitset>

// loosely based on Qflags from Qt

template <typename Enum>
struct enum_size {
	static const size_t value;
};

/*!
 * A typesafe way to define flags as a combination of enum values.
 * 
 * This type should not be used directly, only through DECLARE_FLAGS.
 */
template <typename Enum, size_t Bits = enum_size<Enum>::value>
class flags {
	
public:
	
	typedef Enum enum_type;
	static const size_t bits = Bits;
	typedef std::bitset<bits> Type;
	
private:
	
	typedef void ** Zero;
	typedef void(*TypesafeBoolean)();
	
	Type _flags;
	
	explicit flags(Type flag) : _flags(flag) { }
	
public:
	
	/* implicit */ inline flags(enum_type flag) : _flags(Type().set(size_t(flag))) { }
	
	/* implicit */ inline flags(Zero /* zero */ = 0) : _flags() { }
	
	static flags load(Type _flags) {
		return flags(_flags, true);
	}
	
	//! Test if a specific flag is set.
	bool has(enum_type flag) const {
		return _flags.test(size_t(flag));
	}
	
	//! Test if a collection of flags are all set.
	bool hasAll(flags o) const {
		return (_flags & o._flags) == o._flags;
	}
	
	operator TypesafeBoolean() const {
		return reinterpret_cast<TypesafeBoolean>(_flags.any());
	}
	
	bool operator==(flags o) const {
		return _flags == o._flags;
	}
	
	bool operator!=(flags o) const {
		return _flags != o._flags;
	}
	
	bool operator==(Zero /* zero */) const {
		return _flags == 0;
	}
	
	bool operator!=(Zero /* zero */) const {
		return _flags != 0;
	}
	
	flags operator~() const {
		return flags(~_flags);
	}
	
	bool operator!() const {
		return _flags.none();
	}
	
	flags operator&(flags o) const {
		return flags(_flags & o._flags);
	}
	
	flags operator|(flags o) const {
		return flags(_flags | o._flags);
	}
	
	flags operator^(flags o) const {
		return flags(_flags ^ o._flags);
	}
	
	flags & operator&=(const flags & o) {
		_flags &= o._flags;
		return *this;
	}
	
	flags & operator|=(flags o) {
		_flags |= o._flags;
		return *this;
	}
	
	flags & operator^=(flags o) {
		_flags ^= o._flags;
		return *this;
	}
	
	flags operator&(enum_type flag) const {
		return operator&(flags(flag));
	}
	
	flags operator|(enum_type flag) const {
		return operator|(flags(flag));
	}
	
	flags operator^(enum_type flag) const {
		return operator^(flags(flag));
	}
	
	flags & operator&=(enum_type flag) {
		return operator&=(flags(flag));
	}
	
	flags & operator|=(enum_type flag) {
		return operator|=(flags(flag));
	}
	
	flags & operator^=(enum_type flag) {
		return operator^=(flag);
	}
	
	//! Get a set of flags with all possible values set.
	static flags all() {
		return flags(Type().flip());
	}
	
};

template <typename Enum, size_t Bits>
flags<Enum, Bits> operator|(Enum a, flags<Enum, Bits> b) {
	return b | a;
}

#define DECLARE_ENUM_SIZE(Enum, Size) \
	template <> \
	struct enum_size<Enum> { \
		static const size_t value = (Size); \
	};
#define FLAGS_ENUM_END_HELPER(Enum) Enum ## _End_
#define FLAGS_ENUM_END(Enum) FLAGS_ENUM_END_HELPER(Enum)

/*!
 * Declare overloaded operators for a flag type
 *
 * \param Flagname the flag to declare operators for
 */
#define DECLARE_FLAGS_OPERATORS(Flagname) \
	inline Flagname operator|(Flagname::enum_type a, Flagname::enum_type b) { \
		return Flagname(a) | b; \
	} \
	inline Flagname operator~(Flagname::enum_type a) { \
		return ~Flagname(a); \
	}

//! Get the enum name for a set of flags
#define FLAGS_ENUM(Flagname) Flagname ## _Enum_

/*!
 * Declare a set of flags
 *
 * \param Flagname the name for the flags
 * \param ... the flags to declare
 */
#define FLAGS(Flagname, ...) \
	enum FLAGS_ENUM(Flagname) { \
		__VA_ARGS__, \
		FLAGS_ENUM_END(Flagname) \
	}; \
	typedef ::flags<FLAGS_ENUM(Flagname), FLAGS_ENUM_END(Flagname)> Flagname

/*!
 * Declare overloaded operators and enum_size for a flag type
 *
 * \param Flagname the flag to declare operators for
 */
#define FLAGS_OVERLOADS(Flagname) \
	DECLARE_ENUM_SIZE(FLAGS_ENUM(Flagname), FLAGS_ENUM_END(Flagname)) \
	DECLARE_FLAGS_OPERATORS(Flagname)

#define USE_FLAGS_OVERLOADS(Flagname) \
	(void)(~Flagname::enum_type(0)); \
	(void)(Flagname::enum_type(0) | Flagname::enum_type(0));

#endif // INNOEXTRACT_UTIL_FLAGS_HPP

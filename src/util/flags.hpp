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
	
	inline flags(Type flag) : _flags(flag) { }
	
public:
	
	inline flags(enum_type flag) : _flags(Type().set(size_t(flag))) { }
	
	inline flags(Zero = 0) : _flags() { }
	
	inline flags(const flags & o) : _flags(o._flags) { }
	
	static inline flags load(Type _flags) {
		return flags(_flags, true);
	}
	
	inline bool has(enum_type flag) const {
		return _flags.test(size_t(flag));
	}
	
	inline bool hasAll(flags o) const {
		return (_flags & o._flags) == o._flags;
	}
	
	inline operator TypesafeBoolean() const {
		return reinterpret_cast<TypesafeBoolean>(_flags.any());
	}
	
	inline flags operator~() const {
		return flags(~_flags);
	}
	
	inline bool operator!() const {
		return _flags.none();
	}
	
	inline flags operator&(flags o) const {
		return flags(_flags & o._flags);
	}
	
	inline flags operator|(flags o) const {
		return flags(_flags | o._flags);
	}
	
	inline flags operator^(flags o) const {
		return flags(_flags ^ o._flags);
	}
	
	inline flags & operator&=(const flags & o) {
		_flags &= o._flags;
		return *this;
	}
	
	inline flags & operator|=(flags o) {
		_flags |= o._flags;
		return *this;
	}
	
	inline flags & operator^=(flags o) {
		_flags ^= o._flags;
		return *this;
	}
	
	inline flags operator&(enum_type flag) const {
		return operator&(flags(flag));
	}
	
	inline flags operator|(enum_type flag) const {
		return operator|(flags(flag));
	}
	
	inline flags operator^(enum_type flag) const {
		return operator^(flags(flag));
	}
	
	inline flags & operator&=(enum_type flag) {
		return operator&=(flags(flag));
	}
	
	inline flags & operator|=(enum_type flag) {
		return operator|=(flags(flag));
	}
	
	inline flags & operator^=(enum_type flag) {
		return operator^=(flag);
	}
	
	inline flags & operator=(flags o) {
		_flags = o._flags;
		return *this;
	}
	
	static inline flags all() {
		return flags(Type().flip());
	}
	
};

/*!
 * Declare a flag type using values from a given enum.
 * This should always be used instead of using flags&lt;Enum&gt; directly.
 * 
 * @param Enum should be an enum with values that have exactly one bit set.
 * @param Flagname is the name for the flag type to be defined.
 */
#define DECLARE_FLAGS_SIZE(Enum, Flagname, Size) \
	template <> \
	struct enum_size<Enum> { \
		static const size_t value = (Size); \
	};
#define FLAGS_ENUM_END_HELPER(Enum) Enum ## __End
#define FLAGS_ENUM_END(Enum) FLAGS_ENUM_END_HELPER(Enum)
#define DECLARE_FLAGS(Enum, Flagname) DECLARE_FLAGS_SIZE(Enum, Flagname, FLAGS_ENUM_END(Enum))

/*!
 * Declare overloaded operators for a given flag type.
 */
#define DECLARE_FLAGS_OPERATORS(Flagname) \
	inline Flagname operator|(Flagname::enum_type a, Flagname::enum_type b) { \
		return Flagname(a) | b; \
	} \
	inline Flagname operator|(Flagname::enum_type a, Flagname b) { \
		return b | a; \
	} \
	inline Flagname operator~(Flagname::enum_type a) { \
		return ~Flagname(a); \
	}

#define FLAGS_ENUM(Flagname) Flagname ## __Enum
#define FLAGS(Flagname, ...) \
	enum FLAGS_ENUM(Flagname) { \
		__VA_ARGS__, \
		FLAGS_ENUM_END(Flagname) \
	}; \
	typedef ::flags<FLAGS_ENUM(Flagname), FLAGS_ENUM_END(Flagname)> Flagname
	
#define FLAGS_OVERLOADS(Flagname) \
	DECLARE_FLAGS_SIZE(FLAGS_ENUM(Flagname), Flagname, FLAGS_ENUM_END(Flagname)) \
	DECLARE_FLAGS_OPERATORS(Flagname)

#endif // INNOEXTRACT_UTIL_FLAGS_HPP

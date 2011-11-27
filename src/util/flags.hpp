
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
template <typename _Enum, size_t Bits = enum_size<_Enum>::value>
class flags {
	
public:
	
	typedef _Enum Enum;
	static const size_t bits = Bits;
	typedef std::bitset<bits> Type;
	
private:
	
	typedef void ** Zero;
	typedef void(*TypesafeBoolean)();
	
	Type _flags;
	
	inline flags(Type flag) : _flags(flag) { }
	
public:
	
	inline flags(Enum flag) : _flags(Type().set(size_t(flag))) { }
	
	inline flags(Zero = 0) : _flags() { }
	
	inline flags(const flags & o) : _flags(o._flags) { }
	
	static inline flags load(Type _flags) {
		return flags(_flags, true);
	}
	
	inline bool has(Enum flag) const {
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
	
	inline flags operator&(Enum flag) const {
		return operator&(flags(flag));
	}
	
	inline flags operator|(Enum flag) const {
		return operator|(flags(flag));
	}
	
	inline flags operator^(Enum flag) const {
		return operator^(flags(flag));
	}
	
	inline flags & operator&=(Enum flag) {
		
		return operator&=(flags(flag));
	}
	
	inline flags & operator|=(Enum flag) {
		return operator|=(flags(flag));
	}
	
	inline flags & operator^=(Enum flag) {
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
	inline Flagname operator|(Flagname::Enum a, Flagname::Enum b) { \
		return Flagname(a) | b; \
	} \
	inline Flagname operator|(Flagname::Enum a, Flagname b) { \
		return b | a; \
	} \
	inline Flagname operator~(Flagname::Enum a) { \
		return ~Flagname(a); \
	}
// TODO prevent combination with integers!

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

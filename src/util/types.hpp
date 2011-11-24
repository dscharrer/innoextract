
#ifndef INNOEXTRACT_UTIL_TYPES_HPP
#define INNOEXTRACT_UTIL_TYPES_HPP

#include <limits>

#include <boost/integer/static_min_max.hpp>
#include <boost/integer.hpp>

#include "util/util.hpp"

template <class Base, size_t Bits,
          bool Signed = std::numeric_limits<Base>::is_signed>
struct compatible_integer {
	typedef void type;
};

template <class Base, size_t Bits>
struct compatible_integer<Base, Bits, false> {
	typedef typename boost::uint_t<
		boost::static_unsigned_min<Bits, sizeof(Base) * 8>::value
	>::exact type;
};

template <class Base, size_t Bits>
struct compatible_integer<Base, Bits, true> {
	typedef typename boost::int_t<
		boost::static_unsigned_min<Bits, sizeof(Base) * 8>::value
	>::exact type;
};

template <class T>
inline unsigned int alignment_of() {
#if defined(_MSC_VER) && _MSC_VER >= 1300
	return __alignof(T);
#elif defined(__GNUC__)
	return __alignof__(T);
#else
	return sizeof(T);
#endif
}

inline bool is_aligned_on(const void * p, size_t alignment) {
	return alignment == 1
	       || (is_power_of_2(alignment) ? mod_power_of_2(size_t(p), alignment) == 0
	                                    : size_t(p) % alignment == 0);
}

template <class T>
inline bool is_aligned(const void * p) {
	return is_aligned_on(p, alignment_of<T>());
}

#endif // INNOEXTRACT_UTIL_TYPES_HPP

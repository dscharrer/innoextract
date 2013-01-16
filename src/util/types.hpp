/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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

#ifndef INNOEXTRACT_UTIL_TYPES_HPP
#define INNOEXTRACT_UTIL_TYPES_HPP

#include <limits>

#include <boost/version.hpp>
#include <boost/integer/static_min_max.hpp>
#include <boost/integer.hpp>

#include "util/util.hpp"

template <class Base, size_t Bits,
          bool Signed = std::numeric_limits<Base>::is_signed>
struct compatible_integer {
	typedef void type;
};

#if BOOST_VERSION < 104200

namespace detail {
	
	template <size_t Bits>
	struct uint_t { };
	
	template <>
	struct uint_t<8> : public boost::uint_t<8> {
		typedef uint8_t exact;
	};
	
	template <>
	struct uint_t<16> : public boost::uint_t<16> {
		typedef uint16_t exact;
	};
	
	template <>
	struct uint_t<32> : public boost::uint_t<32> {
		typedef uint32_t exact;
	};
	
	template <>
	struct uint_t<64> {
		typedef uint64_t exact;
	};
	
	template <size_t Bits>
	struct int_t { };
	
	template <>
	struct int_t<8> : public boost::int_t<8> {
		typedef int8_t exact;
	};
	
	template <>
	struct int_t<16> : public boost::int_t<16> {
		typedef int16_t exact;
	};
	
	template <>
	struct int_t<32> : public boost::int_t<32> {
		typedef int32_t exact;
	};
	
	template <>
	struct int_t<64> {
		typedef int64_t exact;
	};
	
}

#else

namespace detail {
	using boost::uint_t;
	using boost::int_t;
}

#endif

template <class Base, size_t Bits>
struct compatible_integer<Base, Bits, false> {
	typedef typename detail::uint_t<
		boost::static_unsigned_min<Bits, sizeof(Base) * 8>::value
	>::exact type;
};

template <class Base, size_t Bits>
struct compatible_integer<Base, Bits, true> {
	typedef typename detail::int_t<
		boost::static_unsigned_min<Bits, sizeof(Base) * 8>::value
	>::exact type;
};

template <class T>
unsigned int alignment_of() {
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
bool is_aligned(const void * p) {
	return is_aligned_on(p, alignment_of<T>());
}

template <class Impl>
class static_polymorphic {
	
protected:
	
	typedef Impl impl_type;
	
	impl_type & impl() { return *static_cast<impl_type *>(this); }
	
	const impl_type & impl() const { return *static_cast<const impl_type *>(this); }
	
};

#endif // INNOEXTRACT_UTIL_TYPES_HPP

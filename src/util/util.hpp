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

#ifndef INNOEXTRACT_UTIL_UTIL_HPP
#define INNOEXTRACT_UTIL_UTIL_HPP

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(*(array)))

template <typename T>
inline T ceildiv(T num, T denom) {
	return (num + (denom - T(1))) / denom;
}

template <class T>
inline bool is_power_of_2(const T & n) {
	return n > 0 && (n & (n - 1)) == 0;
}

template <class T1, class T2>
inline T2 mod_power_of_2(const T1 & a, const T2 & b) {
	return T2(a) & (b - 1);
}

namespace detail {

template <bool overflow>
struct safe_shifter {
	
	template <class T>
	static inline T right_shift(T, unsigned int) {
		return 0;
	}

	template <class T>
	static inline T left_shift(T, unsigned int) {
		return 0;
	}
	
};

template<>
struct safe_shifter<false> {
	
	template <class T>
	static inline T right_shift(T value, unsigned int bits) {
		return value >> bits;
	}

	template <class T>
	static inline T left_shift(T value, unsigned int bits) {
		return value << bits;
	}
	
};

} // namespace detail

template <unsigned int bits, class T>
inline T safe_right_shift(T value) {
	return detail::safe_shifter<(bits >= (8 * sizeof(T)))>::right_shift(value, bits);
}

template <unsigned int bits, class T>
inline T safe_left_shift(T value) {
	return detail::safe_shifter<(bits >= (8 * sizeof(T)))>::left_shift(value, bits);
}

template <class T> inline T rotl_fixed(T x, unsigned int y) {
	return T((x << y) | (x >> (sizeof(T) * 8 - y)));
}

#if defined(_MSC_VER) && _MSC_VER >= 1400 && !defined(__INTEL_COMPILER)

template<> inline uint8_t rotl_fixed<uint8_t>(uint8_t x, unsigned int y) {
	return y ? _rotl8(x, y) : x;
}

// Intel C++ Compiler 10.0 gives undefined externals with these
template<> inline uint16_t rotl_fixed<uint16_t>(uint16_t x, unsigned int y) {
	return y ? _rotl16(x, y) : x;
}

#endif

#ifdef _MSC_VER
template<> inline uint32_t rotl_fixed<uint32_t>(uint32_t x, unsigned int y) {
	return y ? _lrotl(x, y) : x;
}
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1300 && !defined(__INTEL_COMPILER)
// Intel C++ Compiler 10.0 calls a function instead of using the rotate instruction when
// using these instructions
template<> inline uint64_t rotl_fixed<uint64_t>(uint64_t x, unsigned int y) {
	return y ? _rotl64(x, y) : x;
}
#endif

#endif // INNOEXTRACT_UTIL_UTIL_HPP


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

#endif // INNOEXTRACT_UTIL_UTIL_HPP

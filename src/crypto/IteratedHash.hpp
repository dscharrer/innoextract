
#ifndef INNOEXTRACT_CRYPTO_ITERATEDHASH_HPP
#define INNOEXTRACT_CRYPTO_ITERATEDHASH_HPP

// Taken from Crypto++ and modified to fit the project.

#include <stdint.h>
#include <cstring>
#include "crypto/Checksum.hpp"

template <class T>
inline bool isPowerOf2(const T & n) {
	return n > 0 && (n & (n - 1)) == 0;
}

template <class T1, class T2>
inline T2 modPowerOf2(const T1 &a, const T2 &b) {
	return T2(a) & (b-1);
}

template <class T>
inline unsigned int getAlignmentOf() {
#if (_MSC_VER >= 1300)
	return __alignof(T);
#elif defined(__GNUC__)
	return __alignof__(T);
#else
	return sizeof(T);
#endif
}

inline bool isAlignedOn(const void * p, unsigned int alignment) {
	return alignment==1 || (isPowerOf2(alignment) ? modPowerOf2((size_t)p, alignment) == 0 : (size_t)p % alignment == 0);
}

template <class T>
inline bool isAligned(const void * p) {
	return isAlignedOn(p, getAlignmentOf<T>());
}

template <bool overflow> struct SafeShifter;

template<>
struct SafeShifter<true> {
	
	template <class T>
	static inline T RightShift(T value, unsigned int bits) {
		return 0;
	}

	template <class T>
	static inline T LeftShift(T value, unsigned int bits) {
		return 0;
	}
};

template<>
struct SafeShifter<false> {
	
	template <class T>
	static inline T RightShift(T value, unsigned int bits) {
		return value >> bits;
	}

	template <class T>
	static inline T LeftShift(T value, unsigned int bits) {
		return value << bits;
	}
};

template <unsigned int bits, class T>
inline T SafeRightShift(T value) {
	return SafeShifter<(bits>=(8*sizeof(T)))>::RightShift(value, bits);
}

template <unsigned int bits, class T>
inline T SafeLeftShift(T value) {
	return SafeShifter<(bits>=(8*sizeof(T)))>::LeftShift(value, bits);
}

template <class T>
class IteratedHash : public ChecksumBase< IteratedHash<T> > {
	
public:
	
	typedef T Transform;
	typedef typename Transform::HashWord HashWord;
	typedef typename Transform::ByteOrder ByteOrder;
	static const size_t BlockSize = Transform::BlockSize;
	static const size_t HashSize = Transform::HashSize;
	
	inline void init() { countLo = countHi = 0; Transform::init(state); }
	
	void update(const char * data, size_t length);
	
	void finalize(char * result);
	
private:

	size_t hash(const HashWord * input, size_t length);
	void pad(unsigned int lastBlockSize, uint8_t padFirst = 0x80);
	
	inline HashWord getBitCountHi() const {
		return (countLo >> (8 * sizeof(HashWord) - 3)) + (countHi << 3);
	}
	inline HashWord getBitCountLo() const { return countLo << 3; }
	
	HashWord data[BlockSize / sizeof(HashWord)];
	HashWord state[HashSize / sizeof(HashWord)];
	
	HashWord countLo, countHi;
	
};

template <class T>
void IteratedHash<T>::update(const char * input, size_t len) {
	
	HashWord oldCountLo = countLo, oldCountHi = countHi;
	
	if((countLo = oldCountLo + HashWord(len)) < oldCountLo) {
		countHi++; // carry from low to high
	}
	
	countHi += HashWord(SafeRightShift<8 * sizeof(HashWord)>(len));
	
	unsigned int num = modPowerOf2(oldCountLo, size_t(BlockSize));
	uint8_t * d = reinterpret_cast<uint8_t *>(data);
	
	if(num != 0) { // process left over data
		if(num + len >= BlockSize) {
			std::memcpy(d + num, input, BlockSize-num);
			hash(data, BlockSize);
			input += (BlockSize - num);
			len -= (BlockSize - num);
			num = 0;
			// drop through and do the rest
		} else {
			std::memcpy(d + num, input, len);
			return;
		}
	}
	
	// now process the input data in blocks of BlockSize bytes and save the leftovers to m_data
	if(len >= BlockSize) {
		
		if(isAligned<T>(input)) {
			size_t leftOver = hash(reinterpret_cast<const HashWord *>(input), len);
			input += (len - leftOver);
			len = leftOver;
			
		} else {
			do { // copy input first if it's not aligned correctly
				std::memcpy(d, input, BlockSize);
				hash(data, BlockSize);
				input += BlockSize;
				len -= BlockSize;
			} while (len >= BlockSize);
		}
	}

	if(len) {
		memcpy(d, input, len);
	}
}

template <class T>
size_t IteratedHash<T>::hash(const HashWord * input, size_t length) {
	
	do {
		
		if(ByteOrder::native) {
			Transform::transform(state, input);
		} else {
			byteSwap(data, input, BlockSize);
			Transform::transform(state, data);
		}
		
		input += BlockSize / sizeof(HashWord);
		length -= BlockSize;
		
	} while(length >= BlockSize);
	
	return length;
}

template <class T>
void IteratedHash<T>::pad(unsigned int lastBlockSize, uint8_t padFirst) {
	
	unsigned int num = modPowerOf2(countLo, size_t(BlockSize));
	
	uint8_t * d = reinterpret_cast<uint8_t *>(data);
	
	d[num++] = padFirst;
	
	if(num <= lastBlockSize) {
		memset(d + num, 0, lastBlockSize-num);
	} else {
		memset(d+num, 0, BlockSize-num);
		hash(data, BlockSize);
		memset(d, 0, lastBlockSize);
	}
}

inline uint8_t byteSwap(uint8_t value) {
	return value;
}

inline uint16_t byteSwap(uint16_t value) {
#if defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_ushort(value);
#else
	return (uint16_t(uint8_t(value)) << 8) | uint8_t(value >> 8);
#endif
}

inline uint32_t byteSwap(uint32_t value) {
#if defined(__GNUC__)
	return __builtin_bswap32(value);
#elif _MSC_VER >= 1400 || (_MSC_VER >= 1300 && !defined(_DLL))
	return _byteswap_ulong(value);
#else
	return (uint32_t(byteSwap(uint16_t(value))) << 16) | byteSwap(uint16_t(value >> 16));
#endif
}

inline uint64_t byteSwap(uint64_t value) {
#if defined(__GNUC__)
	return __builtin_bswap64(value);
#elif defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_uint64(value);
#else
	return (uint64_t(byteSwap(uint32_t(value))) << 32) | byteSwap(uint32_t(value >> 32));
#endif
}

template <class T>
void byteSwap(T * out, const T * in, size_t byteCount) {
	for(size_t i = 0; i < byteCount / sizeof(T); i++) {
		out[i] = byteSwap(in[i]);
	}
}

template <bool Native>
struct Endianness {
	
	static const size_t native = false;
	
	template <class T>
	static T byteSwapIfAlien(T value) { return byteSwap(value); }
	
	template <class T>
	static void byteSwapIfAlien(const T * in, T * out, size_t byteCount) {
		byteSwap(out, in, byteCount);
	}
	
};

template <>
struct Endianness<true> {
	
	static const size_t native = true;
	
	template <class T>
	static T byteSwapIfAlien(T value) { return value; }
	
	template <class T>
	static void byteSwapIfAlien(const T * in, T * out, size_t byteCount) {
		if(in != out) {
			memcpy(out, in, byteCount);
		}
	}
	
};

struct LittleEndian : public Endianness<true> {
	
	static const size_t offset = 0;
	
};

struct BigEndian : public Endianness<false> {
	
	static const size_t offset = 1;
	
};

template <class T> inline T rotlFixed(T x, unsigned int y) {
	return T((x << y) | (x >> (sizeof(T) * 8 - y)));
}

#if _MSC_VER >= 1400 && !defined(__INTEL_COMPILER)

template<> inline uint8_t rotlFixed<uint8_t>(uint8_t x, unsigned int y) {
	return y ? _rotl8(x, y) : x;
}

// Intel C++ Compiler 10.0 gives undefined externals with these
template<> inline uint16_t rotlFixed<uint16_t>(uint16_t x, unsigned int y) {
	return y ? _rotl16(x, y) : x;
}

#endif

#ifdef _MSC_VER
template<> inline uint32_t rotlFixed<uint32_t>(uint32_t x, unsigned int y) {
	return y ? _lrotl(x, y) : x;
}
#endif

#if _MSC_VER >= 1300 && !defined(__INTEL_COMPILER)
// Intel C++ Compiler 10.0 calls a function instead of using the rotate instruction when using these instructions
template<> inline uint64_t rotlFixed<uint64_t>(uint64_t x, unsigned int y) {
	return y ? _rotl64(x, y) : x;
}
#endif

template <class T>
void IteratedHash<T>::finalize(char * digest) {

	int order = ByteOrder::offset;

	pad(BlockSize - 2 * sizeof(HashWord));
	data[BlockSize / sizeof(HashWord) - 2 + order] = ByteOrder::byteSwapIfAlien(getBitCountLo());
	data[BlockSize / sizeof(HashWord) - 1 - order] = ByteOrder::byteSwapIfAlien(getBitCountHi());

	hash(data, BlockSize);

	if(isAligned<HashWord>(digest) && HashSize % sizeof(HashWord) == 0) {
		ByteOrder::byteSwapIfAlien(state, reinterpret_cast<HashWord *>(digest), HashSize);
	} else {
		ByteOrder::byteSwapIfAlien(state, state, HashSize);
		memcpy(digest, state, HashSize);
	}
	
}

#endif // INNOEXTRACT_CRYPTO_ITERATEDHASH_HPP

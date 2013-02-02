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

#ifndef INNOEXTRACT_CRYPTO_ITERATEDHASH_HPP
#define INNOEXTRACT_CRYPTO_ITERATEDHASH_HPP

// Taken from Crypto++ and modified to fit the project.

#include <cstring>

#include <boost/cstdint.hpp>

#include "crypto/checksum.hpp"
#include "util/endian.hpp"
#include "util/types.hpp"
#include "util/util.hpp"

namespace crypto {

template <class T>
class iterated_hash : public checksum_base< iterated_hash<T> > {
	
public:
	
	typedef T transform;
	typedef typename transform::hash_word hash_word;
	typedef typename transform::byte_order byte_order;
	static const size_t block_size = transform::block_size;
	static const size_t hash_size = transform::hash_size;
	
	void init() { count_lo = count_hi = 0; transform::init(state); }
	
	void update(const char * data, size_t length);
	
	void finalize(char * result);
	
private:

	size_t hash(const hash_word * input, size_t length);
	void pad(size_t last_block_size, boost::uint8_t pad_first = 0x80);
	
	hash_word bit_count_hi() const {
		return (count_lo >> (8 * sizeof(hash_word) - 3)) + (count_hi << 3);
	}
	hash_word bit_count_lo() const { return count_lo << 3; }
	
	hash_word data[block_size / sizeof(hash_word)];
	hash_word state[hash_size / sizeof(hash_word)];
	
	hash_word count_lo, count_hi;
	
};

template <class T>
void iterated_hash<T>::update(const char * input, size_t len) {
	
	hash_word old_count_lo = count_lo;
	
	if((count_lo = old_count_lo + hash_word(len)) < old_count_lo) {
		count_hi++; // carry from low to high
	}
	
	count_hi += hash_word(safe_right_shift<8 * sizeof(hash_word)>(len));
	
	size_t num = mod_power_of_2(old_count_lo, size_t(block_size));
	boost::uint8_t * d = reinterpret_cast<boost::uint8_t *>(data);
	
	if(num != 0) { // process left over data
		if(num + len >= block_size) {
			std::memcpy(d + num, input, block_size-num);
			hash(data, block_size);
			input += (block_size - num);
			len -= (block_size - num);
			// drop through and do the rest
		} else {
			std::memcpy(d + num, input, len);
			return;
		}
	}
	
	// now process the input data in blocks of BlockSize bytes and save the leftovers to m_data
	if(len >= block_size) {
		
		if(is_aligned<T>(input)) {
			size_t leftOver = hash(reinterpret_cast<const hash_word *>(input), len);
			input += (len - leftOver);
			len = leftOver;
			
		} else {
			do { // copy input first if it's not aligned correctly
				std::memcpy(d, input, block_size);
				hash(data, block_size);
				input += block_size;
				len -= block_size;
			} while(len >= block_size);
		}
	}

	if(len) {
		memcpy(d, input, len);
	}
}

template <class T>
size_t iterated_hash<T>::hash(const hash_word * input, size_t length) {
	
	do {
		
		if(byte_order::native) {
			transform::transform(state, input);
		} else {
			byteswap(data, input, block_size);
			transform::transform(state, data);
		}
		
		input += block_size / sizeof(hash_word);
		length -= block_size;
		
	} while(length >= block_size);
	
	return length;
}

template <class T>
void iterated_hash<T>::pad(size_t last_block_size, boost::uint8_t pad_first) {
	
	size_t num = mod_power_of_2(count_lo, size_t(block_size));
	
	boost::uint8_t * d = reinterpret_cast<boost::uint8_t *>(data);
	
	d[num++] = pad_first;
	
	if(num <= last_block_size) {
		memset(d + num, 0, last_block_size - num);
	} else {
		memset(d + num, 0, block_size - num);
		hash(data, block_size);
		memset(d, 0, last_block_size);
	}
}

template <class T>
void iterated_hash<T>::finalize(char * digest) {
	
	size_t order = byte_order::offset;
	
	pad(block_size - 2 * sizeof(hash_word));
	data[block_size / sizeof(hash_word) - 2 + order] = byte_order::byteswap_if_alien(bit_count_lo());
	data[block_size / sizeof(hash_word) - 1 - order] = byte_order::byteswap_if_alien(bit_count_hi());
	
	hash(data, block_size);
	
	if(is_aligned<hash_word>(digest) && hash_size % sizeof(hash_word) == 0) {
		byte_order::byteswap_if_alien(state, reinterpret_cast<hash_word *>(digest), hash_size);
	} else {
		byte_order::byteswap_if_alien(state, state, hash_size);
		memcpy(digest, state, hash_size);
	}
	
}

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_ITERATEDHASH_HPP

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
 * Generic hashing utilities.
 */
#ifndef INNOEXTRACT_CRYPTO_ITERATEDHASH_HPP
#define INNOEXTRACT_CRYPTO_ITERATEDHASH_HPP

// Taken from Crypto++ and modified to fit the project.

#include <cstring>

#include <boost/cstdint.hpp>
#include <boost/range/size.hpp>

#include "crypto/checksum.hpp"
#include "util/align.hpp"
#include "util/endian.hpp"
#include "util/math.hpp"
#include "util/types.hpp"

namespace crypto {

template <class T>
class iterated_hash : public checksum_base< iterated_hash<T> > {
	
public:
	
	typedef T transform;
	typedef typename transform::hash_word hash_word;
	typedef typename transform::byte_order byte_order;
	static const size_t block_size = transform::block_size;
	static const size_t hash_size = transform::hash_size / sizeof(hash_word);
	
	void init() { count_lo = count_hi = 0; transform::init(state); }
	
	void update(const char * data, size_t length);
	
	void finalize(char * result);
	
private:

	size_t hash(const char * input, size_t length);
	void pad(size_t last_block_size, char pad_first = '\x80');
	
	hash_word bit_count_hi() const {
		return (count_lo >> (8 * sizeof(hash_word) - 3)) + (count_hi << 3);
	}
	hash_word bit_count_lo() const { return count_lo << 3; }
	
	char buffer[block_size];
	hash_word state[hash_size];
	
	hash_word count_lo, count_hi;
	
};

template <class T>
void iterated_hash<T>::update(const char * data, size_t length) {
	
	hash_word old_count_lo = count_lo;
	
	if((count_lo = old_count_lo + hash_word(length)) < old_count_lo) {
		count_hi++; // carry from low to high
	}
	
	count_hi += hash_word(util::safe_right_shift<8 * sizeof(hash_word)>(length));
	
	size_t num = util::mod_power_of_2(old_count_lo, size_t(block_size));
	
	if(num != 0) { // process left over data
		if(num + length >= block_size) {
			std::memcpy(buffer + num, data, block_size - num);
			hash(buffer, block_size);
			data += (block_size - num);
			length -= (block_size - num);
			// drop through and do the rest
		} else {
			std::memcpy(buffer + num, data, length);
			return;
		}
	}
	
	// now process the input data in blocks of BlockSize bytes and save the leftovers to m_data
	if(length >= block_size) {
		size_t left_over = hash(data, length);
		data += (length - left_over);
		length = left_over;
	}
	
	if(length) {
		memcpy(buffer, data, length);
	}
}

template <class T>
size_t iterated_hash<T>::hash(const char * input, size_t length) {
	
	if(byte_order::native() && util::is_aligned<T>(input)) {
		
		do {
			
			transform::transform(state, reinterpret_cast<const hash_word *>(input));
			
			input += block_size;
			length -= block_size;
			
		} while(length >= block_size);
		
	} else {
		
		do {
			
			hash_word aligned_buffer[block_size / sizeof(hash_word)];
			byte_order::load(input, aligned_buffer, size_t(boost::size(aligned_buffer)));
			
			transform::transform(state, aligned_buffer);
			
			input += block_size;
			length -= block_size;
			
		} while(length >= block_size);
		
	}
	
	return length;
}

template <class T>
void iterated_hash<T>::pad(size_t last_block_size, char pad_first) {
	
	size_t num = util::mod_power_of_2(count_lo, size_t(block_size));
	
	buffer[num++] = pad_first;
	
	if(num <= last_block_size) {
		memset(buffer + num, 0, last_block_size - num);
	} else {
		memset(buffer + num, 0, block_size - num);
		hash(buffer, block_size);
		memset(buffer, 0, last_block_size);
	}
}

template <class T>
void iterated_hash<T>::finalize(char * result) {
	
	size_t order = transform::offset * sizeof(hash_word);
	
	size_t size = block_size - 2 * sizeof(hash_word);
	pad(size);
	byte_order::store(bit_count_lo(), buffer + size + order);
	byte_order::store(bit_count_hi(), buffer + size + sizeof(hash_word) - order);
	
	hash(buffer, block_size);
	
	byte_order::store(state, hash_size, result);
	
}

} // namespace crypto

#endif // INNOEXTRACT_CRYPTO_ITERATEDHASH_HPP

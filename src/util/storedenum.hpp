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

#ifndef INNOEXTRACT_UTIL_STOREDENUM_HPP
#define INNOEXTRACT_UTIL_STOREDENUM_HPP

#include <stdint.h>
#include <stddef.h>
#include <vector>

#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>

#include "util/enum.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/util.hpp"

template <class Enum>
struct enum_value_map {
	
	typedef Enum enum_type;
	typedef Enum flag_type;
	
};

#define STORED_ENUM_MAP(MapName, Default, ...) \
struct MapName : public enum_value_map<typeof(Default)> { \
	static const flag_type default_value; \
	static const flag_type values[]; \
	static const size_t count; \
}; \
const MapName::flag_type MapName::default_value = Default; \
const MapName::flag_type MapName::values[] = { __VA_ARGS__ }; \
const size_t MapName::count = ARRAY_SIZE(MapName::values)

#define STORED_FLAGS_MAP(MapName, Flag0, ...) \
	STORED_ENUM_MAP(MapName, Flag0, Flag0, ## __VA_ARGS__)

template <class Mapping>
struct stored_enum {
	
	size_t value;
	
public:
	
	typedef Mapping mapping_type;
	typedef typename Mapping::enum_type enum_type;
	
	static const size_t size = Mapping::count;
	
	inline stored_enum(std::istream & is) {
		 // TODO use larger types for larger enums
		BOOST_STATIC_ASSERT(size <= (1 << 8));
		value = load_number<uint8_t>(is);
	}
	
	enum_type get() {
		
		if(value < size) {
			return Mapping::values[value];
		}
		
		log_warning << "warning: unexpected " << enum_names<enum_type>::name << " value: " << value;
		
		return Mapping::default_value;
	}
	
};

template <size_t Bits>
class stored_bitfield {
	
	typedef uint8_t base_type;
	
	static const size_t base_size = sizeof(base_type) * 8;
	static const size_t count = (Bits + (base_size - 1)) / base_size; // ceildiv
	
	base_type bits[count];
	
public:
	
	static const size_t size = Bits;
	
	inline stored_bitfield(std::istream & is) {
		for(size_t i = 0; i < count; i++) {
			bits[i] = load_number<base_type>(is);
		}
	}
	
	inline uint64_t lower_bits() const {
		
		BOOST_STATIC_ASSERT(sizeof(uint64_t) % sizeof(base_type) == 0);
		
		uint64_t result = 0;
		
		for(size_t i = 0; i < std::min(sizeof(uint64_t) / sizeof(base_type), size_t(count)); i++) {
			result |= (uint64_t(bits[i]) << (i * base_size));
		}
		
		return result;
	}
	
	inline operator std::bitset<size>() const {
		
		// Make `make style` shut up since we really need unsigned long here.
		#define stored_enum_concat_(a, b, c, d) a##b c##d
		typedef stored_enum_concat_(unsi, gned, lo, ng) ulong_type;
		#undef stored_enum_concat_
		
		static const size_t ulong_size = sizeof(ulong_type) * 8;
		
		BOOST_STATIC_ASSERT(base_size % ulong_size == 0 || base_size < ulong_size);
		
		std::bitset<size> result(0);
		for(size_t i = 0; i < count; i++) {
			for(size_t j = 0; j < ceildiv(base_size, ulong_size); j++) {
				result |= std::bitset<size>(static_cast<ulong_type>(bits[i] >> (j * ulong_size)))
				          << ((i * base_size) + (j * ulong_size));
			}
		}
		return result;
	}
	
};

template <class Mapping>
class stored_flags : private stored_bitfield<Mapping::count> {
	
public:
	
	typedef Mapping mapping_type;
	typedef typename Mapping::enum_type enum_type;
	typedef flags<enum_type> flag_type;
	
	inline stored_flags(std::istream & is) : stored_bitfield<Mapping::count>(is) { }
	
	flag_type get() {
		
		uint64_t bits = this->lower_bits();
		flag_type result = 0;
		
		for(size_t i = 0; i < this->size; i++) {
			if(bits & (uint64_t(1) << i)) {
				result |= Mapping::values[i];
				bits &= ~(uint64_t(1) << i);
			}
		}
		
		if(bits) {
			log_warning << "unexpected " << enum_names<enum_type>::name << " flags: "
			            << std::hex << bits << std::dec;
		}
		
		return result;
	}
	
};

template <class Enum>
class stored_flag_reader {
	
public:
	
	typedef Enum enum_type;
	typedef flags<enum_type> flag_type;
	
	std::istream & is;
	
	typedef uint8_t stored_type;
	static const size_t stored_bits = sizeof(stored_type) * 8;
	
	size_t pos;
	stored_type buffer;
	
	flag_type result;
	
	size_t bits;
	
	explicit stored_flag_reader(std::istream & _is) : is(_is), pos(0), result(0), bits(0) { }
	
	void add(enum_type flag) {
		
		if(pos == 0) {
			buffer = load_number<stored_type>(is);
		}
		
		if(buffer & (stored_type(1) << pos)) {
			result |= flag;
		}
		
		pos = (pos + 1) % stored_bits;
		
		bits++;
	}
	
	operator flag_type() const {
		return result;
	}
	
};

template <class Enum>
class stored_flag_reader<flags<Enum> > : public stored_flag_reader<Enum> {
	
public:
	
	explicit stored_flag_reader(std::istream & is) : stored_flag_reader<Enum>(is) { }
	
};

typedef stored_bitfield<256> stored_char_set;

#endif // INNOEXTRACT_UTIL_STOREDENUM_HPP

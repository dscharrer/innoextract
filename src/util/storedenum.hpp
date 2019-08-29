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
 * Utilities for decoding stored enum values into run-time values.
 */
#ifndef INNOEXTRACT_UTIL_STOREDENUM_HPP
#define INNOEXTRACT_UTIL_STOREDENUM_HPP

#include <stddef.h>
#include <vector>
#include <bitset>
#include <ios>

#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/utility/enable_if.hpp>

#include "util/enum.hpp"
#include "util/load.hpp"
#include "util/log.hpp"

// Shared info for enums and flags
#define STORED_MAP_HELPER(MapName, TypeRep, DefaultDecl, ...) \
struct MapName { \
	typedef BOOST_TYPEOF(TypeRep) enum_type; \
	DefaultDecl \
	static const enum_type values[]; \
	static const size_t count; \
}; \
const MapName::enum_type MapName::values[] = { __VA_ARGS__ }; \
const size_t MapName::count = (sizeof(MapName::values)/sizeof(*(MapName::values)))

//! Declare a mapping from integers to enum elements to be used for \ref stored_enum
#define STORED_ENUM_MAP(MapName, Default, /* elements */ ...) \
	STORED_MAP_HELPER(MapName, Default, \
	static const enum_type default_value;, \
	__VA_ARGS__); \
const MapName::enum_type MapName::default_value = Default

//! Declare a mapping from bits to flag enum elements to be used for \ref stored_flags
#define STORED_FLAGS_MAP(MapName, Flag0, /* additional flags */ ...) \
	STORED_MAP_HELPER(MapName, Flag0, , Flag0, __VA_ARGS__)

template <class Mapping>
struct stored_enum {
	
	size_t value;
	
public:
	
	typedef Mapping mapping_type;
	typedef typename Mapping::enum_type enum_type;
	
	static const size_t size = Mapping::count;
	
	explicit stored_enum(std::istream & is) {
		BOOST_STATIC_ASSERT(size <= (1 << 8));
		value = util::load<boost::uint8_t>(is);
	}
	
	enum_type get() {
		
		if(value < size) {
			return Mapping::values[value];
		}
		
		log_warning << "Unexpected " << enum_names<enum_type>::name << " value: " << value;
		
		return Mapping::default_value;
	}
	
};

/*!
 * Load a packed bitfield: 1 byte for every 8 bits
 * The only exception is that 3-byte bitfields are padded to 4 bytes for non-16-bit builds.
 */
template <size_t Bits, size_t PadBits = 32>
class stored_bitfield {
	
	typedef boost::uint8_t base_type;
	
	static const size_t base_size = sizeof(base_type) * 8;
	static const size_t count = (Bits + (base_size - 1)) / base_size; // ceildiv
	
	base_type bits[count];
	
public:
	
	static const size_t size = Bits;
	
	explicit stored_bitfield(std::istream & is) {
		for(size_t i = 0; i < count; i++) {
			bits[i] = util::load<base_type>(is);
		}
		if(count == 3 && PadBits == 32) {
			// 3-byte sets are padded to 4 bytes
			(void)util::load<base_type>(is);
		}
	}
	
	boost::uint64_t lower_bits() const {
		
		BOOST_STATIC_ASSERT(sizeof(boost::uint64_t) % sizeof(base_type) == 0);
		
		boost::uint64_t result = 0;
		
		for(size_t i = 0; i < std::min(sizeof(boost::uint64_t) / sizeof(base_type), size_t(count)); i++) {
			result |= (boost::uint64_t(bits[i]) << (i * base_size));
		}
		
		return result;
	}
	
	operator std::bitset<size>() const {
		
		#define concat(a, b) a##b
		BOOST_STATIC_ASSERT(sizeof(base_type) <= sizeof(concat(unsi, gned) concat(lo, ng)));
		#undef concat
		
		std::bitset<size> result(0);
		for(size_t i = 0; i < count; i++) {
			result |= std::bitset<size>(bits[i]) << (i * base_size);
		}
		return result;
	}
	
};

/*!
 * Load a flag set where the possible flags are known at compile-time.
 * Inno Setup stores flag sets as packed bitfields: 1 byte for every 8 flags
 * The only exception is that 3-byte bitfields are padded to 4 bytes for non-16-bit builds.
 */
template <class Mapping, size_t PadBits = 32>
class stored_flags : private stored_bitfield<Mapping::count, PadBits> {
	
public:
	
	typedef Mapping mapping_type;
	typedef typename Mapping::enum_type enum_type;
	typedef flags<enum_type> flag_type;
	
	explicit stored_flags(std::istream & is)
		: stored_bitfield<Mapping::count, PadBits>(is) { }
	
	flag_type get() {
		
		boost::uint64_t set_bits = this->lower_bits();
		flag_type result = 0;
		
		for(size_t i = 0; i < this->size; i++) {
			if(set_bits & (boost::uint64_t(1) << i)) {
				result |= Mapping::values[i];
				set_bits &= ~(boost::uint64_t(1) << i);
			}
		}
		
		if(set_bits) {
			log_warning << "Unexpected " << enum_names<enum_type>::name << " flags: "
			            << std::hex << set_bits << std::dec;
		}
		
		return result;
	}
	
};

/*!
 * Load a flag set where the possible flags are not known at compile-time.
 * Inno Setup stores flag sets as packed bitfields: 1 byte for every 8 flags
 * The only exception is that 3-byte bitfields are padded to 4 bytes for non-16-bit builds.
 */
template <class Enum>
class stored_flag_reader {
	
public:
	
	typedef Enum enum_type;
	typedef flags<enum_type> flag_type;
	
private:
	
	const size_t pad_bits;
	
	std::istream & stream;
	
	typedef boost::uint8_t stored_type;
	static const size_t stored_bits = sizeof(stored_type) * 8;
	
	size_t pos;
	stored_type buffer;
	
	flag_type result;
	
	size_t bytes;
	
public:
	
	explicit stored_flag_reader(std::istream & is, size_t padding_bits = 32)
		: pad_bits(padding_bits), stream(is), pos(0), buffer(0), result(0), bytes(0) { }
	
	//! Declare the next possible flag.
	void add(enum_type flag) {
		
		if(pos == 0) {
			bytes++;
			buffer = util::load<stored_type>(stream);
		}
		
		if(buffer & (stored_type(1) << pos)) {
			result |= flag;
		}
		
		pos = (pos + 1) % stored_bits;
	}
	
	operator flag_type() const {
		if(bytes == 3 && pad_bits == 32) {
			// 3-byte sets are padded to 4 bytes
			(void)util::load<stored_type>(stream);
		}
		return result;
	}
	
};

template <class Enum>
class stored_flag_reader<flags<Enum> > : public stored_flag_reader<Enum> {
	
public:
	
	explicit stored_flag_reader(std::istream & is, size_t padding_bits = 32)
		: stored_flag_reader<Enum>(is, padding_bits) { }
	
};

typedef stored_bitfield<256> stored_char_set;

#endif // INNOEXTRACT_UTIL_STOREDENUM_HPP


#ifndef INNOEXTRACT_UTIL_STOREDENUM_HPP
#define INNOEXTRACT_UTIL_STOREDENUM_HPP

#include <stdint.h>
#include <stddef.h>
#include <vector>

#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>

#include "util/Enum.hpp"
#include "util/LoadingUtils.hpp"
#include "util/Output.hpp"

template <class Enum>
struct EnumValueMap {
	
	typedef Enum enum_type;
	typedef Enum flag_type;
	
};

#define STORED_ENUM_MAP(MapName, Default, ...) \
struct MapName : public EnumValueMap<typeof(Default)> { \
	static const flag_type default_value; \
	static const flag_type values[]; \
	static const size_t count; \
}; \
const MapName::flag_type MapName::default_value = Default; \
const MapName::flag_type MapName::values[] = { __VA_ARGS__ }; \
const size_t MapName::count = ARRAY_SIZE(MapName::values)

#define STORED_FLAGS_MAP(MapName, Flag0, ...) STORED_ENUM_MAP(MapName, Flag0, Flag0, ## __VA_ARGS__)

template <class Mapping>
struct StoredEnum {
	
	size_t value;
	
public:
	
	typedef Mapping mapping_type;
	typedef typename Mapping::enum_type enum_type;
	
	static const size_t size = Mapping::count;
	
	inline StoredEnum(std::istream & is) {
		value = loadNumber<uint8_t>(is); // TODO use larger types for larger enums
		std::cout << "[e]read: " << PrintHex(value) << " - " << size << std::endl;
	}
	
	enum_type get() {
		
		if(value < size) {
			return Mapping::values[value];
		}
		
		LogWarning << "warning: unexpected " << EnumNames<enum_type>::name << " value: " << value;
		
		return Mapping::default_value;
	}
	
};

template <size_t Bits>
class StoredBitfield {
	
	typedef uint8_t base_type;
	
	static const size_t base_size = sizeof(base_type) * 8;
	static const size_t count = (Bits + (base_size - 1)) / base_size; // ceildiv
	
	base_type bits[count];
	
public:
	
	static const size_t size = Bits;
	
	inline StoredBitfield(std::istream & is) {
		for(size_t i = 0; i < count; i++) {
			bits[i] = loadNumber<base_type>(is);
		}
	}
	
	inline uint64_t getLowerBits() const {
		
		BOOST_STATIC_ASSERT(sizeof(uint64_t) % sizeof(base_type) == 0);
		
		uint64_t result = 0;
		
		for(size_t i = 0; i < std::min(sizeof(uint64_t) / sizeof(base_type), size_t(count)); i++) {
			result |= (uint64_t(bits[i]) << (i * base_size));
		}
		
		return result;
	}
	
	inline std::bitset<size> getBitSet() const {
		
		static const size_t ulong_size = sizeof(unsigned long) * 8;
		
		BOOST_STATIC_ASSERT(base_size % ulong_size == 0 || base_size < ulong_size);
		
		std::bitset<size> result(0);
		for(size_t i = 0; i < count; i++) {
			for(size_t j = 0; j < ceildiv(base_size, ulong_size); j++) {
				result |= std::bitset<size>(static_cast<unsigned long>(bits[i] >> (j * ulong_size)))
				          << ((i * base_size) + (j * ulong_size));
			}
		}
		return result;
	}
	
};

template <class Mapping>
class StoredFlags : private StoredBitfield<Mapping::count> {
	
public:
	
	typedef Mapping mapping_type;
	typedef typename Mapping::enum_type enum_type;
	typedef Flags<enum_type> flag_type;
	
	inline StoredFlags(std::istream & is) : StoredBitfield<Mapping::count>(is) { }
	
	flag_type get() {
		
		uint64_t bits = this->getLowerBits();
		flag_type result = 0;
		
		for(size_t i = 0; i < this->size; i++) {
			if(bits & (uint64_t(1) << i)) {
				result |= Mapping::values[i];
				bits &= ~(uint64_t(1) << i);
			}
		}
		
		if(bits) {
			LogWarning << "unexpected " << EnumNames<enum_type>::name << " flags: " << std::hex << bits << std::dec;
		}
		
		return result;
	}
	
};

template <class Enum>
class StoredFlagReader {
	
public:
	
	typedef Enum enum_type;
	typedef Flags<enum_type> flag_type;
	
	std::istream & is;
	
	typedef uint8_t stored_type;
	static const size_t stored_bits = sizeof(stored_type) * 8;
	
	size_t pos;
	stored_type buffer;
	
	flag_type result;
	
	size_t bits;
	
	StoredFlagReader(std::istream & _is) : is(_is), pos(0), result(0), bits(0) { };
	
	void add(enum_type flag) {
		
		if(pos == 0) {
			buffer = loadNumber<stored_type>(is);
			std::cout << "[f]read: " << PrintHex(int(buffer)) << std::endl;
		}
		
		if(buffer & (stored_type(1) << pos)) {
			result |= flag;
		}
		
		pos = (pos + 1) % stored_bits;
		
		bits++;
	}
	
	flag_type get() {
		return result;
	}
	
};

template <class Enum>
class StoredFlagReader<Flags<Enum> > : public StoredFlagReader<Enum> {
	
public:
	
	StoredFlagReader(std::istream & is) : StoredFlagReader<Enum>(is) { };
	
};

typedef StoredBitfield<256> CharSet;

#endif // INNOEXTRACT_UTIL_STOREDENUM_HPP

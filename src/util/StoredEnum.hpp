
#ifndef INNOEXTRACT_UTIL_STOREDENUM_HPP
#define INNOEXTRACT_UTIL_STOREDENUM_HPP

#include <vector>

#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>

#include "util/Enum.hpp"
#include "util/LoadingUtils.hpp"
#include "util/Output.hpp"
#include "util/Types.hpp"

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
	
	u32 value;
	
public:
	
	typedef Mapping mapping_type;
	typedef typename Mapping::enum_type enum_type;
	
	static const size_t size = Mapping::count;
	
	inline StoredEnum(std::istream & is) {
		value = loadNumber<u8>(is); // TODO use larger types for larger enums
	}
	
	enum_type get() {
		
		if(value < size) {
			return Mapping::values[value];
		}
		
		warning << "warning: unexpected " << EnumNames<enum_type>::name << " value: " << value;
		
		return Mapping::default_value;
	}
	
};

template <size_t Bits>
class StoredBitfield {
	
	typedef u8 base_type;
	
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
	
	inline u64 getLowerBits() const {
		
		BOOST_STATIC_ASSERT(sizeof(u64) % sizeof(base_type) == 0);
		
		u64 result = 0;
		
		for(size_t i = 0; i < std::min(sizeof(u64) / sizeof(base_type), count); i++) {
			result |= (u64(bits[i]) << (i * base_size));
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
		
		u64 bits = this->getLowerBits();
		flag_type result = 0;
		
		for(size_t i = 0; i < this->size; i++) {
			if(bits & (u64(1) << i)) {
				result |= Mapping::values[i];
				bits &= ~(u64(1) << i);
			}
		}
		
		if(bits) {
			warning << "unexpected " << EnumNames<enum_type>::name << " flags: " << std::hex << bits << std::dec;
		}
		
		return result;
	}
	
};

template <class Enum>
class StoredFlagReader {
	
public:
	
	typedef Enum enum_type;
	typedef Flags<enum_type> flag_type;
	
	std::vector<enum_type> mappings;
	
	void add(enum_type flag) {
		mappings.push_back(flag);
	}
	
	
	flag_type get(std::istream & is) {
		
		u64 bits = 0;
		
		typedef u8 stored_type;
		static const size_t stored_bits = sizeof(stored_type) * 8;
		for(size_t i = 0; i < ceildiv(mappings.size(), stored_bits); i++) {
			bits |= u64(load<stored_type>(is)) << (i * stored_bits);
		}
		
		flag_type result = 0;
		
		for(size_t i = 0; i < mappings.size(); i++) {
			if(bits & (u64(1) << i)) {
				result |= mappings[i];
				bits &= ~(u64(1) << i);
			}
		}
		
		if(bits) {
			warning << "unexpected " << EnumNames<enum_type>::name << " flags: " << std::hex << bits << std::dec;
		}
		
		return result;
	}
	
};

template <class Enum>
class StoredFlagReader<Flags<Enum> > : public StoredFlagReader<Enum> { };

typedef StoredBitfield<256> CharSet;

#endif // INNOEXTRACT_UTIL_STOREDENUM_HPP

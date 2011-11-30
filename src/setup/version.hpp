
#ifndef INNOEXTRACT_SETUP_VERSION_HPP
#define INNOEXTRACT_SETUP_VERSION_HPP

#include <stdint.h>
#include <iosfwd>
#include <exception>

namespace setup {

struct version_error : public std::exception { };

typedef uint32_t version_constant;
#define INNO_VERSION_EXT(a, b, c, d) ( \
	  (::setup::version_constant(a) << 24) \
	| (::setup::version_constant(b) << 16) \
	| (::setup::version_constant(c) <<  8) \
	| (::setup::version_constant(d)      ) \
)
#define INNO_VERSION(a, b, c) INNO_VERSION_EXT(a, b, c, 0)

struct version {
	
	version_constant value;
	
	uint8_t bits; // 16 or 32
	
	bool unicode;
	
	bool known;
	
	inline version() : known(false) { }
	
	inline version(version_constant value, bool unicode = false,
	               bool known = false, uint8_t bits = 32)
		: value(value), bits(bits), unicode(unicode), known(known) { }
	
	
	inline version(uint8_t a, uint8_t b, uint8_t c, uint8_t d = 0, bool unicode = false,
	               bool known = false, uint8_t bits = 32)
		: value(INNO_VERSION_EXT(a, b, c, d)), bits(bits), unicode(unicode), known(known) { }
	
	inline unsigned int a() const { return  value >> 24;         }
	inline unsigned int b() const { return (value >> 16) & 0xff; }
	inline unsigned int c() const { return (value >>  8) & 0xff; }
	inline unsigned int d() const { return  value        & 0xff; }
	
	void load(std::istream & is);
	
	//! @return the Windows codepage used to encode strings
	inline uint32_t codepage() const { return uint32_t(unicode ? 1200 : 1252); }
	
	//! @return true if the version stored might not be correct
	inline bool is_ambiguous() const;
	
	operator version_constant() const {
		return value;
	}
	
	version_constant next();
	
};

std::ostream & operator<<(std::ostream & os, const version & version);

} // namespace setup

#endif // INNOEXTRACT_SETUP_VERSION_HPP

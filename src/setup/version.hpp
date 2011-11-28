
#ifndef INNOEXTRACT_SETUP_VERSION_HPP
#define INNOEXTRACT_SETUP_VERSION_HPP

#include <stdint.h>
#include <iosfwd>

typedef uint32_t InnoVersionConstant;
#define INNO_VERSION_EXT(a, b, c, d) \
	((InnoVersionConstant(a) << 24) | (InnoVersionConstant(b) << 16) \
	 | (InnoVersionConstant(c) << 8) | InnoVersionConstant(d))
#define INNO_VERSION(a, b, c) INNO_VERSION_EXT(a, b, c, 0)

struct inno_version {
	
	InnoVersionConstant version;
	
	uint8_t bits; // 16 or 32
	
	bool unicode;
	
	bool known;
	
	
	inline inno_version() : known(false) { }
	
	inline inno_version(InnoVersionConstant _version, bool _unicode = false,
	                   bool _known = false, uint8_t _bits = 32)
		: version(_version), bits(_bits), unicode(_unicode), known(_known) { }
	
	
	inline inno_version(uint8_t a, uint8_t b, uint8_t c, uint8_t d = 0, bool _unicode = false,
	                   bool _known = false, uint8_t _bits = 32)
		: version(INNO_VERSION_EXT(a, b, c, d)), bits(_bits), unicode(_unicode), known(_known) { }
	
	inline unsigned int a() const { return version >> 24; }
	inline unsigned int b() const { return (version >> 16) & 0xff; }
	inline unsigned int c() const { return (version >> 8) & 0xff; }
	inline unsigned int d() const { return version & 0xff; }
	
	void load(std::istream & is);
	
	//! @return the Windows codepage used for encode strings
	inline uint32_t codepage() const { return uint32_t(unicode ? 1200 : 1252); }
	
	//! @return true if the version stored might not be correct
	inline bool isSuspicious() const;
	
};

inline bool operator==(const inno_version & a, const inno_version & b) {
	return a.version == b.version;
}
inline bool operator!=(const inno_version & a, const inno_version & b) {
	return !operator==(a, b);
}
inline bool operator< (const inno_version & a, const inno_version & b) {
	return a.version < b.version;
}
inline bool operator> (const inno_version & a, const inno_version & b) {
	return  operator< (b, a);
}
inline bool operator<=(const inno_version & a, const inno_version & b) {
	return !operator> (a, b);
}
inline bool operator>=(const inno_version & a, const inno_version & b) {
	return !operator< (a, b);
}

inline bool operator==(const inno_version & a, InnoVersionConstant b) { return a.version == b;    }
inline bool operator!=(const inno_version & a, InnoVersionConstant b) { return !operator==(a, b); }
inline bool operator< (const inno_version & a, InnoVersionConstant b) { return a.version < b;     }
inline bool operator> (const inno_version & a, InnoVersionConstant b) { return  operator< (b, a); }
inline bool operator<=(const inno_version & a, InnoVersionConstant b) { return !operator> (a, b); }
inline bool operator>=(const inno_version & a, InnoVersionConstant b) { return !operator< (a, b); }

inline bool operator==(InnoVersionConstant a, const inno_version & b) { return a == b.version;    }
inline bool operator!=(InnoVersionConstant a, const inno_version & b) { return !operator==(a, b); }
inline bool operator< (InnoVersionConstant a, const inno_version & b) { return a < b.version;     }
inline bool operator> (InnoVersionConstant a, const inno_version & b) { return  operator< (b, a); }
inline bool operator<=(InnoVersionConstant a, const inno_version & b) { return !operator> (a, b); }
inline bool operator>=(InnoVersionConstant a, const inno_version & b) { return !operator< (a, b); }

std::ostream & operator<<(std::ostream & os, const inno_version & version);

#endif // INNOEXTRACT_SETUP_VERSION_HPP

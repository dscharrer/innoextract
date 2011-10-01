
#ifndef INNOEXTRACT_VERSION_HPP
#define INNOEXTRACT_VERSION_HPP

#include <iostream>
#include <utility>

#include "Types.hpp"

typedef u32 InnoVersionConstant;
#define INNO_VERSION_EXT(a, b, c, d) ((u32(a) << 24) | (u32(b) << 16) | (u32(c) << 8) | u32(d))
#define INNO_VERSION(a, b, c) INNO_VERSION_EXT(a, b, c, 0)

struct InnoVersion {
	
	InnoVersionConstant version;
	
	char bits; // 16 or 32
	
	bool unicode;
	
	bool known;
	
	
	inline InnoVersion() : known(false) { };
	
	inline InnoVersion(InnoVersionConstant _version, bool _unicode = false, bool _known = false, char _bits = 32) : version(_version), unicode(_unicode), known(_known), bits(_bits) { };
	
	
	inline InnoVersion(char a, char b, char c, char d = 0, bool _unicode = false, bool _known = false, char _bits = 32) : version(INNO_VERSION_EXT(a, b, c, d)), unicode(_unicode), known(_known), bits(_bits) { };
	
	inline int a() const { return version >> 24; }
	inline int b() const { return (version >> 16) & 0xff; }
	inline int c() const { return (version >> 8) & 0xff; }
	inline int d() const { return version & 0xff; }
	
	void load(std::istream & is);
	
	inline u32 codepage() const { return u32(unicode ? 1200 : 1252); }
	
	/*!
	 * @return true if the version stored might not be correct
	 */
	inline bool isSuspicious() const;
	
};

inline bool operator==(const InnoVersion & a, const InnoVersion & b) { return a.version == b.version; }
inline bool operator!=(const InnoVersion & a, const InnoVersion & b) { return !operator==(a, b);      }
inline bool operator< (const InnoVersion & a, const InnoVersion & b) { return a.version < b.version;  }
inline bool operator> (const InnoVersion & a, const InnoVersion & b) { return  operator< (b, a);      }
inline bool operator<=(const InnoVersion & a, const InnoVersion & b) { return !operator> (a, b);      }
inline bool operator>=(const InnoVersion & a, const InnoVersion & b) { return !operator< (a, b);      }

inline bool operator==(const InnoVersion & a, InnoVersionConstant b) { return a.version == b;    }
inline bool operator!=(const InnoVersion & a, InnoVersionConstant b) { return !operator==(a, b); }
inline bool operator< (const InnoVersion & a, InnoVersionConstant b) { return a.version < b;     }
inline bool operator> (const InnoVersion & a, InnoVersionConstant b) { return  operator< (b, a); }
inline bool operator<=(const InnoVersion & a, InnoVersionConstant b) { return !operator> (a, b); }
inline bool operator>=(const InnoVersion & a, InnoVersionConstant b) { return !operator< (a, b); }

inline bool operator==(InnoVersionConstant a, const InnoVersion & b) { return a == b.version;    }
inline bool operator!=(InnoVersionConstant a, const InnoVersion & b) { return !operator==(a, b); }
inline bool operator< (InnoVersionConstant a, const InnoVersion & b) { return a < b.version;     }
inline bool operator> (InnoVersionConstant a, const InnoVersion & b) { return  operator< (b, a); }
inline bool operator<=(InnoVersionConstant a, const InnoVersion & b) { return !operator> (a, b); }
inline bool operator>=(InnoVersionConstant a, const InnoVersion & b) { return !operator< (a, b); }

std::ostream & operator<<(std::ostream & os, const InnoVersion & version);

#endif // INNOEXTRACT_VERSION_HPP

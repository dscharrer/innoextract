
#include "setup/Version.hpp"

#include <cstring>

#include <boost/static_assert.hpp>

#include <util/Utils.hpp>

typedef char StoredLegacySetupDataVersion[12];

struct KnownLegacySetupDataVersion {
	
	char name[13]; // terminating 0 byte is ignored
	
	InnoVersionConstant version;
	
	unsigned char bits;
	
};

const KnownLegacySetupDataVersion knownLegacySetupDataVersions[] = {
	{ "i1.2.10--16\x1a", INNO_VERSION(1, 2, 10), 16 },
	{ "i1.2.10--32\x1a", INNO_VERSION(1, 2, 10), 32 },
};

typedef char StoredSetupDataVersion[64];

struct KnownSetupDataVersion {
	
	StoredSetupDataVersion name;
	
	InnoVersionConstant version;
	bool unicode;
	
};

const KnownSetupDataVersion knownSetupDataVersions[] = {
	{ "Inno Setup Setup Data (1.3.21)",                INNO_VERSION_EXT(1, 3, 21, 0) },
	{ "Inno Setup Setup Data (1.3.25)",                INNO_VERSION_EXT(1, 3, 25, 0) },
	{ "Inno Setup Setup Data (2.0.0)",                 INNO_VERSION_EXT(2, 0,  0, 0) },
	{ "Inno Setup Setup Data (2.0.1)",                 INNO_VERSION_EXT(2, 0,  1, 0) }, // or 2.0.2!
	{ "Inno Setup Setup Data (2.0.5)",                 INNO_VERSION_EXT(2, 0,  5, 0) },
	{ "Inno Setup Setup Data (2.0.6a)",                INNO_VERSION_EXT(2, 0,  6, 0) },
	{ "Inno Setup Setup Data (2.0.7)",                 INNO_VERSION_EXT(2, 0,  7, 0) },
	{ "Inno Setup Setup Data (2.0.8)",                 INNO_VERSION_EXT(2, 0,  8, 0) },
	{ "Inno Setup Setup Data (2.0.11)",                INNO_VERSION_EXT(2, 0, 11, 0) },
	{ "Inno Setup Setup Data (2.0.17)",                INNO_VERSION_EXT(2, 0, 17, 0) },
	{ "Inno Setup Setup Data (2.0.18)",                INNO_VERSION_EXT(2, 0, 18, 0) },
	{ "Inno Setup Setup Data (3.0.0a)",                INNO_VERSION_EXT(3, 0,  0, 0) },
	{ "Inno Setup Setup Data (3.0.1)",                 INNO_VERSION_EXT(3, 0,  1, 0) },
	{ "Inno Setup Setup Data (3.0.3)",                 INNO_VERSION_EXT(3, 0,  3, 0) }, // or 3.0.4!
	{ "Inno Setup Setup Data (3.0.5)",                 INNO_VERSION_EXT(3, 0,  5, 0) },
	{ "My Inno Setup Extensions Setup Data (3.0.6.1)", INNO_VERSION_EXT(3, 0,  6, 1) },
	{ "Inno Setup Setup Data (4.0.0a)",                INNO_VERSION_EXT(4, 0,  0, 0) },
	{ "Inno Setup Setup Data (4.0.1)",                 INNO_VERSION_EXT(4, 0,  1, 0) },
	{ "Inno Setup Setup Data (4.0.3)",                 INNO_VERSION_EXT(4, 0,  3, 0) },
	{ "Inno Setup Setup Data (4.0.5)",                 INNO_VERSION_EXT(4, 0,  5, 0) },
	{ "Inno Setup Setup Data (4.0.9)",                 INNO_VERSION_EXT(4, 0,  9, 0) },
	{ "Inno Setup Setup Data (4.0.10)",                INNO_VERSION_EXT(4, 0, 10, 0) },
	{ "Inno Setup Setup Data (4.0.11)",                INNO_VERSION_EXT(4, 0, 11, 0) },
	{ "Inno Setup Setup Data (4.1.0)",                 INNO_VERSION_EXT(4, 1,  0, 0) },
	{ "Inno Setup Setup Data (4.1.2)",                 INNO_VERSION_EXT(4, 1,  2, 0) },
	{ "Inno Setup Setup Data (4.1.3)",                 INNO_VERSION_EXT(4, 1,  3, 0) },
	{ "Inno Setup Setup Data (4.1.4)",                 INNO_VERSION_EXT(4, 1,  4, 0) },
	{ "Inno Setup Setup Data (4.1.5)",                 INNO_VERSION_EXT(4, 1,  5, 0) },
	{ "Inno Setup Setup Data (4.1.6)",                 INNO_VERSION_EXT(4, 1,  6, 0) },
	{ "Inno Setup Setup Data (4.1.8)",                 INNO_VERSION_EXT(4, 1,  8, 0) },
	{ "Inno Setup Setup Data (4.2.0)",                 INNO_VERSION_EXT(4, 2,  0, 0) },
	{ "Inno Setup Setup Data (4.2.1)",                 INNO_VERSION_EXT(4, 2,  1, 0) },
	{ "Inno Setup Setup Data (4.2.2)",                 INNO_VERSION_EXT(4, 2,  2, 0) },
	{ "Inno Setup Setup Data (4.2.3)",                 INNO_VERSION_EXT(4, 2,  3, 0) }, // or 4.2.4!
	{ "Inno Setup Setup Data (4.2.5)",                 INNO_VERSION_EXT(4, 2,  5, 0) },
	{ "Inno Setup Setup Data (4.2.6)",                 INNO_VERSION_EXT(4, 2,  6, 0) },
	{ "Inno Setup Setup Data (5.0.0)",                 INNO_VERSION_EXT(5, 0,  0, 0) },
	{ "Inno Setup Setup Data (5.0.1)",                 INNO_VERSION_EXT(5, 0,  1, 0) },
	{ "Inno Setup Setup Data (5.0.3)",                 INNO_VERSION_EXT(5, 0,  3, 0) },
	{ "Inno Setup Setup Data (5.0.4)",                 INNO_VERSION_EXT(5, 0,  4, 0) },
	{ "Inno Setup Setup Data (5.1.0)",                 INNO_VERSION_EXT(5, 1,  0, 0) },
	{ "Inno Setup Setup Data (5.1.2)",                 INNO_VERSION_EXT(5, 1,  2, 0) },
	{ "Inno Setup Setup Data (5.1.7)",                 INNO_VERSION_EXT(5, 1,  7, 0) },
	{ "Inno Setup Setup Data (5.1.10)",                INNO_VERSION_EXT(5, 1, 10, 0) },
	{ "Inno Setup Setup Data (5.1.13)",                INNO_VERSION_EXT(5, 1, 13, 0) },
	{ "Inno Setup Setup Data (5.2.0)",                 INNO_VERSION_EXT(5, 2,  0, 0) },
	{ "Inno Setup Setup Data (5.2.1)",                 INNO_VERSION_EXT(5, 2,  1, 0) },
	{ "Inno Setup Setup Data (5.2.3)",                 INNO_VERSION_EXT(5, 2,  3, 0) },
	{ "Inno Setup Setup Data (5.2.5)",                 INNO_VERSION_EXT(5, 2,  5, 0) },
	{ "Inno Setup Setup Data (5.2.5) (u)",             INNO_VERSION_EXT(5, 2,  5, 0), true },
	{ "Inno Setup Setup Data (5.3.0)",                 INNO_VERSION_EXT(5, 3,  0, 0) },
	{ "Inno Setup Setup Data (5.3.0) (u)",             INNO_VERSION_EXT(5, 3,  0, 0), true },
	{ "Inno Setup Setup Data (5.3.3)",                 INNO_VERSION_EXT(5, 3,  3, 0) },
	{ "Inno Setup Setup Data (5.3.3) (u)",             INNO_VERSION_EXT(5, 3,  3, 0), true },
	{ "Inno Setup Setup Data (5.3.5)",                 INNO_VERSION_EXT(5, 3,  5, 0) },
	{ "Inno Setup Setup Data (5.3.5) (u)",             INNO_VERSION_EXT(5, 3,  5, 0), true },
	{ "Inno Setup Setup Data (5.3.6)",                 INNO_VERSION_EXT(5, 3,  6, 0) },
	{ "Inno Setup Setup Data (5.3.6) (u)",             INNO_VERSION_EXT(5, 3,  6, 0), true },
	{ "Inno Setup Setup Data (5.3.7)",                 INNO_VERSION_EXT(5, 3,  7, 0) },
	{ "Inno Setup Setup Data (5.3.7) (u)",             INNO_VERSION_EXT(5, 3,  7, 0), true },
	{ "Inno Setup Setup Data (5.3.8)",                 INNO_VERSION_EXT(5, 3,  8, 0) },
	{ "Inno Setup Setup Data (5.3.8) (u)",             INNO_VERSION_EXT(5, 3,  8, 0), true },
	{ "Inno Setup Setup Data (5.3.9)",                 INNO_VERSION_EXT(5, 3,  9, 0) },
	{ "Inno Setup Setup Data (5.3.9) (u)",             INNO_VERSION_EXT(5, 3,  9, 0), true },
	{ "Inno Setup Setup Data (5.3.10)",                INNO_VERSION_EXT(5, 3, 10, 0) },
	{ "Inno Setup Setup Data (5.3.10) (u)",            INNO_VERSION_EXT(5, 3, 10, 0), true },
	{ "Inno Setup Setup Data (5.4.2)",                 INNO_VERSION_EXT(5, 4,  2, 0) },
	{ "Inno Setup Setup Data (5.4.2) (u)",             INNO_VERSION_EXT(5, 4,  2, 0), true },
};
using std::cout;
using std::string;
using std::endl;

std::ostream & operator<<(std::ostream & os, const InnoVersion & v) {
	
	os << v.a() << '.' << v.b() << '.' << v.c();
	if(v.d()) {
		os << '.' << v.d();
	}
	
	if(v.unicode) {
		os << " (unicode)";
	}
	
	if(v.bits != 32) {
		os << " (" << int(v.bits) << "-bit)";
	}
	
	if(!v.known) {
		os << " [unsupported]";
	}
	
	return os;
}

void InnoVersion::load(std::istream & is) {
	
	BOOST_STATIC_ASSERT(sizeof(StoredLegacySetupDataVersion) <= sizeof(StoredSetupDataVersion));
	
	StoredLegacySetupDataVersion legacyVersion;
	is.read(legacyVersion, sizeof(legacyVersion));
	
	if(legacyVersion[0] == 'i' && legacyVersion[sizeof(legacyVersion) - 1] == '\x1a') {
		
		cout << "found legacy version: \"" << safestring(legacyVersion, sizeof(legacyVersion) - 1) << '"' << endl;
		
		for(size_t i = 0; i < ARRAY_SIZE(knownLegacySetupDataVersions); i++) {
			if(!memcmp(legacyVersion, knownLegacySetupDataVersions[i].name, sizeof(legacyVersion))) {
				version = knownLegacySetupDataVersions[i].version;
				bits = knownLegacySetupDataVersions[i].bits;
				unicode = false;
				known = true;
				cout << "-> version is known" << endl;
				return;
			}
		}
		
		// TODO autodetect version
		
		known = false;
		
		cout << "-> unknown version" << endl;
		throw new string("bad version");
	}
	
	StoredSetupDataVersion storedVersion;
	memcpy(storedVersion, legacyVersion, sizeof(legacyVersion));
	is.read(storedVersion + sizeof(legacyVersion), sizeof(storedVersion) - sizeof(legacyVersion));
	
	cout << "found version: \"" << safestring(storedVersion) << '"' << endl;
	
	for(size_t i = 0; i < ARRAY_SIZE(knownSetupDataVersions); i++) {
		if(!memcmp(storedVersion, knownSetupDataVersions[i].name, sizeof(storedVersion))) {
			version = knownSetupDataVersions[i].version;
			bits = 32;
			unicode = knownSetupDataVersions[i].unicode;
			known = true;
			cout << "-> version is known" << endl;
			return;
		}
	}
	
	// TODO autodetect version
	
	known = false;
	
	cout << "-> unknown version" << endl;
	throw new string("bad version");
}

bool InnoVersion::isSuspicious() const {
	
	if(version == INNO_VERSION(2, 0, 1)) {
		// might be either 2.0.1 or 2.0.2
		return true;
	}
	
	if(version == INNO_VERSION(3, 0, 3)) {
		// might be either 3.0.3 or 3.0.4
		return true;
	}
	
	if(version == INNO_VERSION(4, 2, 3)) {
		// might be either 4.2.3 or 4.2.4
		return true;
	}
	
	return false;
}

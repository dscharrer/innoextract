
#include "setup/WindowsVersion.hpp"

#include <stdint.h>
#include "util/LoadingUtils.hpp"
#include "util/Utils.hpp"

const WindowsVersion WindowsVersion::none = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0 } };

void WindowsVersion::Version::load(std::istream& is, const InnoVersion& version) {
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		build = loadNumber<uint16_t>(is);
	} else {
		build = 0;
	}
	
	minor = loadNumber<uint8_t>(is);
	major = loadNumber<uint8_t>(is);
	
}

void WindowsVersion::load(std::istream & is, const InnoVersion & version) {
	
	winVersion.load(is, version);
	ntVersion.load(is, version);
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		ntServicePack.minor = loadNumber<uint8_t>(is);
		ntServicePack.major = loadNumber<uint8_t>(is);
	} else {
		ntServicePack.major = 0, ntServicePack.minor = 0;
	}
	
}

namespace {

struct WindowsVersionName {
	
	const char * name;
	
	WindowsVersion::Version version;
	
};

WindowsVersionName windowsVersionNames[] = {
	{ "Windows 1.0", { 1, 4, 0 } },
	{ "Windows 2.0", { 2, 11, 0 } },
	{ "Windows 3.0", { 3, 0, 0 } },
	{ "Windows for Workgroups 3.11", { 3, 11, 0 } },
	{ "Windows 95", { 4, 0, 950 } },
	{ "Windows 98", { 4, 1, 1998 } },
	{ "Windows 98 Second Edition", { 4, 1, 2222 } },
	{ "Windows ME", { 4, 90, 3000 } },
};

WindowsVersionName windowsNtVersionNames[] = {
	{ "Windows NT Workstation 3.5", { 3, 5, 807 } },
	{ "Windows NT 3.1", { 3, 10, 528 } },
	{ "Windows NT Workstation 3.51", { 3, 51, 1057 } },
	{ "Windows NT Workstation 4.0", { 4, 0, 1381 } },
	{ "Windows 2000", { 5, 0, 2195 } },
	{ "Windows XP", { 5, 1, 2600 } },
	{ "Windows XP x64", { 5, 2, 3790 } },
	{ "Windows Vista", { 6, 0, 6000 } },
	{ "Windows 7", { 6, 1, 7600 } }
};

const char * getVersionName(const WindowsVersion::Version & version, bool nt = false) {
	
	WindowsVersionName * names;
	size_t count;
	if(nt) {
		names = windowsNtVersionNames, count = ARRAY_SIZE(windowsNtVersionNames);
	} else {
		names = windowsVersionNames, count = ARRAY_SIZE(windowsVersionNames);
	}
	
	for(size_t i = 0; i < count; i++) {
		const WindowsVersionName & v = names[i];
		if(v.version.major != version.major || v.version.minor < version.minor) {
			continue;
		}
		return v.name;
	};
	return NULL;
}

}

std::ostream & operator<<(std::ostream & os, const WindowsVersion::Version & v) {
	os << v.major << '.' << v.minor;
	if(v.build) {
		os << v.build;
	}
}

std::ostream & operator<<(std::ostream & os, const WindowsVersion & v) {
	os << v.winVersion;
	if(v.ntVersion != v.winVersion) {
		os << "  nt " << v.ntVersion;
	}
	const char * winName = getVersionName(v.winVersion);
	const char * ntName = getVersionName(v.ntVersion, true);
	if(winName || ntName) {
		os << " (";
		if(winName) {
			os << winName;
		}
		if(ntName && ntName != winName) {
			if(winName) {
				os << " / ";
			}
			os << ntName;
		}
		os << ')';
	}
	if(v.ntServicePack.major || v.ntServicePack.minor) {
	 os << " service pack " << v.ntServicePack.major;
	 if(v.ntServicePack.minor) {
		 os << '.' << v.ntServicePack.minor;
	 }
	}
	return os;
}

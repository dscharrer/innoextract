
#include "WindowsVersion.hpp"

#include "LoadingUtils.hpp"
#include "Utils.hpp"

const WindowsVersion WindowsVersion::none = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0 } };

void WindowsVersion::Version::load(std::istream& is, const InnoVersion& version) {
	
	if(version > INNO_VERSION(1, 2, 16)) {
		build = loadNumber<u16>(is);
	} else {
		build = 0;
	}
	
	minor = loadNumber<u8>(is);
	major = loadNumber<u8>(is);
	
}

void WindowsVersion::load(std::istream & is, const InnoVersion & version) {
	
	winVersion.load(is, version);
	ntVersion.load(is, version);
	
	if(version > INNO_VERSION(1, 2, 16)) {
		ntServicePack.minor = loadNumber<u8>(is);
		ntServicePack.major = loadNumber<u8>(is);
	} else {
		ntServicePack.major = 0, ntServicePack.minor = 0;
	}
	
}

namespace {

struct WindowsVersionName {
	
	const char * name;
	
	WindowsVersion::Version version;
	
	bool nt;
	
};

WindowsVersionName windowsVersionNames[] = {
	{ "Windows 1.0", { 1, 4, 0 } },
	{ "Windows 2.0", { 2, 11, 0 } },
	{ "Windows 3.0", { 3, 0, 0 } },
	{ "Windows NT Workstation 3.5", { 3, 5, 807 }, true },
	{ "Windows NT 3.1", { 3, 10, 528 }, true },
	{ "Windows for Workgroups 3.11", { 3, 11, 0 } },
	{ "Windows NT Workstation 3.51", { 3, 51, 1057 }, true },
	{ "Windows 95", { 4, 0, 950 } },
	{ "Windows NT Workstation 4.0", { 4, 0, 1381 }, true },
	{ "Windows 98", { 4, 1, 1998 } },
	{ "Windows 98 Second Edition", { 4, 1, 2222 } },
	{ "Windows ME", { 4, 90, 3000 } },
	{ "Windows 2000", { 5, 0, 2195 }, true },
	{ "Windows XP", { 5, 1, 2600 }, true },
	{ "Windows XP x64", { 5, 2, 3790 }, true },
	{ "Windows Vista", { 6, 0, 6000 }, true },
	{ "Windows 7", { 6, 1, 7600 }, true }
};

const char * getVersionName(const WindowsVersion::Version & version, bool nt = false) {
	for(size_t i = 0; i < ARRAY_SIZE(windowsVersionNames); i++) {
		const WindowsVersionName & v = windowsVersionNames[i];
		if(v.version.major != version.major || v.version.minor < version.minor) {
			continue;
		}
		if(nt != v.nt) {
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

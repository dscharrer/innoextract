
#include "setup/windows.hpp"

#include <stdint.h>
#include <ostream>

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/util.hpp"

namespace setup {

const windows_version windows_version::none = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0 } };

void windows_version::data::load(std::istream & is, const version & version) {
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		build = load_number<uint16_t>(is);
	} else {
		build = 0;
	}
	
	minor = load_number<uint8_t>(is);
	major = load_number<uint8_t>(is);
	
}

void windows_version::load(std::istream & is, const version & version) {
	
	win_version.load(is, version);
	nt_version.load(is, version);
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		nt_service_pack.minor = load_number<uint8_t>(is);
		nt_service_pack.major = load_number<uint8_t>(is);
	} else {
		nt_service_pack.major = 0, nt_service_pack.minor = 0;
	}
	
}

void windows_version_range::load(std::istream & is, const version & version) {
	begin.load(is, version);
	end.load(is, version);
}


namespace {

struct windows_version_name {
	
	const char * name;
	
	windows_version::data version;
	
};

windows_version_name windows_version_names[] = {
	{ "Windows 1.0", { 1, 4, 0 } },
	{ "Windows 2.0", { 2, 11, 0 } },
	{ "Windows 3.0", { 3, 0, 0 } },
	{ "Windows for Workgroups 3.11", { 3, 11, 0 } },
	{ "Windows 95", { 4, 0, 950 } },
	{ "Windows 98", { 4, 1, 1998 } },
	{ "Windows 98 Second Edition", { 4, 1, 2222 } },
	{ "Windows ME", { 4, 90, 3000 } },
};

windows_version_name windows_nt_version_names[] = {
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

const char * get_version_name(const windows_version::data & version, bool nt = false) {
	
	windows_version_name * names;
	size_t count;
	if(nt) {
		names = windows_nt_version_names, count = ARRAY_SIZE(windows_nt_version_names);
	} else {
		names = windows_version_names, count = ARRAY_SIZE(windows_version_names);
	}
	
	for(size_t i = 0; i < count; i++) {
		const windows_version_name & v = names[i];
		if(v.version.major != version.major || v.version.minor < version.minor) {
			continue;
		}
		return v.name;
	};
	return NULL;
}

} // nanonymous namespace

std::ostream & operator<<(std::ostream & os, const windows_version::data & v) {
	os << v.major << '.' << v.minor;
	if(v.build) {
		os << v.build;
	}
	return os;
}

std::ostream & operator<<(std::ostream & os, const windows_version & v) {
	
	os << v.win_version;
	if(v.nt_version != v.win_version) {
		os << "  nt " << v.nt_version;
	}
	
	const char * win_name = get_version_name(v.win_version);
	const char * nt_name = get_version_name(v.nt_version, true);
	
	if(win_name || nt_name) {
		os << " (";
		if(win_name) {
			os << win_name;
		}
		if(nt_name && nt_name != win_name) {
			if(win_name) {
				os << " / ";
			}
			os << nt_name;
		}
		os << ')';
	}
	
	if(v.nt_service_pack.major || v.nt_service_pack.minor) {
		os << " service pack " << v.nt_service_pack.major;
		if(v.nt_service_pack.minor) {
			os << '.' << v.nt_service_pack.minor;
		}
	}
	
	return os;
}

} // namespace setup

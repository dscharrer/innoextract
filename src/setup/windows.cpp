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

#include "setup/windows.hpp"

#include <ostream>

#include <boost/cstdint.hpp>
#include <boost/range/size.hpp>

#include "setup/version.hpp"
#include "util/load.hpp"

namespace setup {

const windows_version windows_version::none = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0 } };

void windows_version::data::load(std::istream & is, const version & version) {
	
	if(version >= INNO_VERSION(1, 3, 19)) {
		build = util::load<boost::uint16_t>(is);
	} else {
		build = 0;
	}
	
	minor = util::load<boost::uint8_t>(is);
	major = util::load<boost::uint8_t>(is);
	
}

void windows_version::load(std::istream & is, const version & version) {
	
	win_version.load(is, version);
	nt_version.load(is, version);
	
	if(version >= INNO_VERSION(1, 3, 19)) {
		nt_service_pack.minor = util::load<boost::uint8_t>(is);
		nt_service_pack.major = util::load<boost::uint8_t>(is);
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
	{ "Windows 7", { 6, 1, 7600 } },
	{ "Windows 8", { 6, 2, 0 } },
	{ "Windows 8.1", { 6, 3, 0 } },
	{ "Windows 10", { 10, 0, 0 } },
};

const char * get_version_name(const windows_version::data & version, bool nt = false) {
	
	windows_version_name * names;
	size_t count;
	if(nt) {
		names = windows_nt_version_names, count = size_t(boost::size(windows_nt_version_names));
	} else {
		names = windows_version_names, count = size_t(boost::size(windows_version_names));
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

} // anonymous namespace

std::ostream & operator<<(std::ostream & os, const windows_version::data & version) {
	os << version.major << '.' << version.minor;
	if(version.build) {
		os << version.build;
	}
	return os;
}

std::ostream & operator<<(std::ostream & os, const windows_version & version) {
	
	os << version.win_version;
	if(version.nt_version != version.win_version) {
		os << "  nt " << version.nt_version;
	}
	
	const char * win_name = get_version_name(version.win_version);
	const char * nt_name = get_version_name(version.nt_version, true);
	
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
	
	if(version.nt_service_pack.major || version.nt_service_pack.minor) {
		os << " service pack " << version.nt_service_pack.major;
		if(version.nt_service_pack.minor) {
			os << '.' << version.nt_service_pack.minor;
		}
	}
	
	return os;
}

} // namespace setup

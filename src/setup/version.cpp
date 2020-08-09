/*
 * Copyright (C) 2011-2020 Daniel Scharrer
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

#include "setup/version.hpp"

#include <cstring>
#include <algorithm>
#include <istream>
#include <ostream>

#include <boost/version.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/algorithm/string.hpp"
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>

#include "util/load.hpp"
#include "util/log.hpp"

namespace setup {

namespace {

typedef char stored_legacy_version[12];

struct known_legacy_version {
	
	char name[13]; // terminating 0 byte is ignored
	
	version_constant version;
	version::flags variant;
	
	operator version_constant() const { return version; }
	
};

const known_legacy_version legacy_versions[] = {
	{ "i1.2.10--16\x1a", INNO_VERSION(1, 2, 10), version::Bits16 },
	{ "i1.2.10--32\x1a", INNO_VERSION(1, 2, 10), 0 },
};

typedef char stored_version[64];

struct known_version {
	
	stored_version name;
	
	version_constant version;
	version::flags variant;
	
	operator version_constant() const { return version; }
	
};

const known_version versions[] = {
	{ "Inno Setup Setup Data (1.3.3)",                      INNO_VERSION_EXT(1, 3,  3, 0), 0 },
	{ "Inno Setup Setup Data (1.3.9)",                      INNO_VERSION_EXT(1, 3,  9, 0), 0 },
	{ "Inno Setup Setup Data (1.3.10)",                     INNO_VERSION_EXT(1, 3, 10, 0), 0 },
	{ "Inno Setup Setup Data (1.3.10) with ISX (1.3.10)",   INNO_VERSION_EXT(1, 3, 10, 0), version::ISX },
	{ "Inno Setup Setup Data (1.3.12) with ISX (1.3.12.1)", INNO_VERSION_EXT(1, 3, 12, 1), version::ISX },
	{ "Inno Setup Setup Data (1.3.21)",     /* ambiguous */ INNO_VERSION_EXT(1, 3, 21, 0), 0 },
	{ "Inno Setup Setup Data (1.3.21) with ISX (1.3.17)",   INNO_VERSION_EXT(1, 3, 21, 0), version::ISX },
	{ "Inno Setup Setup Data (1.3.24)",                     INNO_VERSION_EXT(1, 3, 24, 0), 0 },
	{ "Inno Setup Setup Data (1.3.21) with ISX (1.3.24)",   INNO_VERSION_EXT(1, 3, 24, 0), version::ISX },
	{ "Inno Setup Setup Data (1.3.25)",                     INNO_VERSION_EXT(1, 3, 25, 0), 0 },
	{ "Inno Setup Setup Data (1.3.25) with ISX (1.3.25)",   INNO_VERSION_EXT(1, 3, 25, 0), version::ISX },
	{ "Inno Setup Setup Data (2.0.0)",                      INNO_VERSION_EXT(2, 0,  0, 0), 0 },
	{ "Inno Setup Setup Data (2.0.1)",      /* ambiguous */ INNO_VERSION_EXT(2, 0,  1, 0), 0 },
	{ "Inno Setup Setup Data (2.0.2)",                      INNO_VERSION_EXT(2, 0,  2, 0), 0 },
	{ "Inno Setup Setup Data (2.0.5)",                      INNO_VERSION_EXT(2, 0,  5, 0), 0 },
	{ "Inno Setup Setup Data (2.0.6a)",                     INNO_VERSION_EXT(2, 0,  6, 0), 0 },
	{ "Inno Setup Setup Data (2.0.6a) with ISX (2.0.3)",    INNO_VERSION_EXT(2, 0,  6, 0), version::ISX },
	{ "Inno Setup Setup Data (2.0.7)",                      INNO_VERSION_EXT(2, 0,  7, 0), 0 },
	{ "Inno Setup Setup Data (2.0.8)",                      INNO_VERSION_EXT(2, 0,  8, 0), 0 },
	{ "Inno Setup Setup Data (2.0.8) with ISX (2.0.3)",     INNO_VERSION_EXT(2, 0,  8, 0), version::ISX },
	{ "Inno Setup Setup Data (2.0.8) with ISX (2.0.10)",    INNO_VERSION_EXT(2, 0, 10, 0), version::ISX },
	{ "Inno Setup Setup Data (2.0.11)",                     INNO_VERSION_EXT(2, 0, 11, 0), 0 },
	{ "Inno Setup Setup Data (2.0.11) with ISX (2.0.11)",   INNO_VERSION_EXT(2, 0, 11, 0), version::ISX },
	{ "Inno Setup Setup Data (2.0.17)",                     INNO_VERSION_EXT(2, 0, 17, 0), 0 },
	{ "Inno Setup Setup Data (2.0.17) with ISX (2.0.11)",   INNO_VERSION_EXT(2, 0, 17, 0), version::ISX },
	{ "Inno Setup Setup Data (2.0.18)",                     INNO_VERSION_EXT(2, 0, 18, 0), 0 },
	{ "Inno Setup Setup Data (2.0.18) with ISX (2.0.11)",   INNO_VERSION_EXT(2, 0, 18, 0), version::ISX },
	{ "Inno Setup Setup Data (3.0.0a)",                     INNO_VERSION_EXT(3, 0,  0, 0), 0 },
	{ "Inno Setup Setup Data (3.0.1)",                      INNO_VERSION_EXT(3, 0,  1, 0), 0 },
	{ "Inno Setup Setup Data (3.0.1) with ISX (3.0.0)",     INNO_VERSION_EXT(3, 0,  1, 0), version::ISX },
	{ "Inno Setup Setup Data (3.0.3)",      /* ambiguous */ INNO_VERSION_EXT(3, 0,  3, 0), 0 },
	{ "Inno Setup Setup Data (3.0.3) with ISX (3.0.3)",     INNO_VERSION_EXT(3, 0,  3, 0), version::ISX },
	{ "Inno Setup Setup Data (3.0.4)",                      INNO_VERSION_EXT(3, 0,  4, 0), 0 },
	{ "My Inno Setup Extensions Setup Data (3.0.4)",        INNO_VERSION_EXT(3, 0,  4, 0), version::ISX },
	{ "Inno Setup Setup Data (3.0.5)",                      INNO_VERSION_EXT(3, 0,  5, 0), 0 },
	{ "My Inno Setup Extensions Setup Data (3.0.6.1)",      INNO_VERSION_EXT(3, 0,  6, 1), version::ISX },
	{ "Inno Setup Setup Data (4.0.0a)",                     INNO_VERSION_EXT(4, 0,  0, 0), 0 },
	{ "Inno Setup Setup Data (4.0.1)",                      INNO_VERSION_EXT(4, 0,  1, 0), 0 },
	{ "Inno Setup Setup Data (4.0.3)",                      INNO_VERSION_EXT(4, 0,  3, 0), 0 },
	{ "Inno Setup Setup Data (4.0.5)",                      INNO_VERSION_EXT(4, 0,  5, 0), 0 },
	{ "Inno Setup Setup Data (4.0.9)",                      INNO_VERSION_EXT(4, 0,  9, 0), 0 },
	{ "Inno Setup Setup Data (4.0.10)",                     INNO_VERSION_EXT(4, 0, 10, 0), 0 },
	{ "Inno Setup Setup Data (4.0.11)",                     INNO_VERSION_EXT(4, 0, 11, 0), 0 },
	{ "Inno Setup Setup Data (4.1.0)",                      INNO_VERSION_EXT(4, 1,  0, 0), 0 },
	{ "Inno Setup Setup Data (4.1.2)",                      INNO_VERSION_EXT(4, 1,  2, 0), 0 },
	{ "Inno Setup Setup Data (4.1.3)",                      INNO_VERSION_EXT(4, 1,  3, 0), 0 },
	{ "Inno Setup Setup Data (4.1.4)",                      INNO_VERSION_EXT(4, 1,  4, 0), 0 },
	{ "Inno Setup Setup Data (4.1.5)",                      INNO_VERSION_EXT(4, 1,  5, 0), 0 },
	{ "Inno Setup Setup Data (4.1.6)",                      INNO_VERSION_EXT(4, 1,  6, 0), 0 },
	{ "Inno Setup Setup Data (4.1.8)",                      INNO_VERSION_EXT(4, 1,  8, 0), 0 },
	{ "Inno Setup Setup Data (4.2.0)",                      INNO_VERSION_EXT(4, 2,  0, 0), 0 },
	{ "Inno Setup Setup Data (4.2.1)",                      INNO_VERSION_EXT(4, 2,  1, 0), 0 },
	{ "Inno Setup Setup Data (4.2.2)",                      INNO_VERSION_EXT(4, 2,  2, 0), 0 },
	{ "Inno Setup Setup Data (4.2.3)",      /* ambiguous */ INNO_VERSION_EXT(4, 2,  3, 0), 0 },
	{ "Inno Setup Setup Data (4.2.4)",                      INNO_VERSION_EXT(4, 2,  4, 0), 0 },
	{ "Inno Setup Setup Data (4.2.5)",                      INNO_VERSION_EXT(4, 2,  5, 0), 0 },
	{ "Inno Setup Setup Data (4.2.6)",                      INNO_VERSION_EXT(4, 2,  6, 0), 0 },
	{ "Inno Setup Setup Data (5.0.0)",                      INNO_VERSION_EXT(5, 0,  0, 0), 0 },
	{ "Inno Setup Setup Data (5.0.1)",                      INNO_VERSION_EXT(5, 0,  1, 0), 0 },
	{ "Inno Setup Setup Data (5.0.3)",                      INNO_VERSION_EXT(5, 0,  3, 0), 0 },
	{ "Inno Setup Setup Data (5.0.4)",                      INNO_VERSION_EXT(5, 0,  4, 0), 0 },
	{ "Inno Setup Setup Data (5.1.0)",                      INNO_VERSION_EXT(5, 1,  0, 0), 0 },
	{ "Inno Setup Setup Data (5.1.2)",                      INNO_VERSION_EXT(5, 1,  2, 0), 0 },
	{ "Inno Setup Setup Data (5.1.7)",                      INNO_VERSION_EXT(5, 1,  7, 0), 0 },
	{ "Inno Setup Setup Data (5.1.10)",                     INNO_VERSION_EXT(5, 1, 10, 0), 0 },
	{ "Inno Setup Setup Data (5.1.13)",                     INNO_VERSION_EXT(5, 1, 13, 0), 0 },
	{ "Inno Setup Setup Data (5.2.0)",                      INNO_VERSION_EXT(5, 2,  0, 0), 0 },
	{ "Inno Setup Setup Data (5.2.1)",                      INNO_VERSION_EXT(5, 2,  1, 0), 0 },
	{ "Inno Setup Setup Data (5.2.3)",                      INNO_VERSION_EXT(5, 2,  3, 0), 0 },
	{ "Inno Setup Setup Data (5.2.5)",                      INNO_VERSION_EXT(5, 2,  5, 0), 0 },
	{ "Inno Setup Setup Data (5.2.5) (u)",                  INNO_VERSION_EXT(5, 2,  5, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.3.0)",                      INNO_VERSION_EXT(5, 3,  0, 0), 0 },
	{ "Inno Setup Setup Data (5.3.0) (u)",                  INNO_VERSION_EXT(5, 3,  0, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.3.3)",                      INNO_VERSION_EXT(5, 3,  3, 0), 0 },
	{ "Inno Setup Setup Data (5.3.3) (u)",                  INNO_VERSION_EXT(5, 3,  3, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.3.5)",                      INNO_VERSION_EXT(5, 3,  5, 0), 0 },
	{ "Inno Setup Setup Data (5.3.5) (u)",                  INNO_VERSION_EXT(5, 3,  5, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.3.6)",                      INNO_VERSION_EXT(5, 3,  6, 0), 0 },
	{ "Inno Setup Setup Data (5.3.6) (u)",                  INNO_VERSION_EXT(5, 3,  6, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.3.7)",                      INNO_VERSION_EXT(5, 3,  7, 0), 0 },
	{ "Inno Setup Setup Data (5.3.7) (u)",                  INNO_VERSION_EXT(5, 3,  7, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.3.8)",                      INNO_VERSION_EXT(5, 3,  8, 0), 0 },
	{ "Inno Setup Setup Data (5.3.8) (u)",                  INNO_VERSION_EXT(5, 3,  8, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.3.9)",                      INNO_VERSION_EXT(5, 3,  9, 0), 0 },
	{ "Inno Setup Setup Data (5.3.9) (u)",                  INNO_VERSION_EXT(5, 3,  9, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.3.10)",                     INNO_VERSION_EXT(5, 3, 10, 0), 0 },
	{ "Inno Setup Setup Data (5.3.10) (u)",                 INNO_VERSION_EXT(5, 3, 10, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.4.2)",                      INNO_VERSION_EXT(5, 4,  2, 0), 0 },
	{ "Inno Setup Setup Data (5.4.2) (u)",  /* ambiguous */ INNO_VERSION_EXT(5, 4,  2, 0), version::Unicode },
	{ "" /* BlackBox v1? */,                                INNO_VERSION_EXT(5, 4,  2, 1), 0 },
	{ "" /* BlackBox v1? */,                                INNO_VERSION_EXT(5, 4,  2, 1), version::Unicode },
	{ "Inno Setup Setup Data (5.5.0)",                      INNO_VERSION_EXT(5, 5,  0, 0), 0 },
	{ "Inno Setup Setup Data (5.5.0) (u)",  /* ambiguous */ INNO_VERSION_EXT(5, 5,  0, 0), version::Unicode },
	{ "" /* BlackBox v2 / Inno Setup Ultra */,              INNO_VERSION_EXT(5, 5,  0, 1), 0 },
	{ "" /* BlackBox v2 / Inno Setup Ultra */,              INNO_VERSION_EXT(5, 5,  0, 1), version::Unicode },
	{ "Inno Setup Setup Data (5.5.6)",                      INNO_VERSION_EXT(5, 5,  6, 0), 0 },
	{ "Inno Setup Setup Data (5.5.6) (u)",                  INNO_VERSION_EXT(5, 5,  6, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.5.7)",      /* ambiguous */ INNO_VERSION_EXT(5, 5,  7, 0), 0 },
	{ "Inno Setup Setup Data (5.5.7) (u)",  /* ambiguous */ INNO_VERSION_EXT(5, 5,  7, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.5.7) (U)",  /* ambiguous */ INNO_VERSION_EXT(5, 5,  7, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.5.8) (u)", /* unofficial */ INNO_VERSION_EXT(5, 5,  7, 0), version::Unicode },
	{ "" /* unknown 5.5.7 (u) variant */,   /* ambiguous */ INNO_VERSION_EXT(5, 5,  7, 1), 0 },
	{ "" /* unknown 5.5.7 (u) variant */,   /* ambiguous */ INNO_VERSION_EXT(5, 5,  7, 1), version::Unicode },
	{ "Inno Setup Setup Data (5.6.0)",                      INNO_VERSION_EXT(5, 6,  0, 0), 0 },
	{ "Inno Setup Setup Data (5.6.0) (u)",                  INNO_VERSION_EXT(5, 6,  0, 0), version::Unicode },
	{ "Inno Setup Setup Data (5.6.2)",     /* prerelease */ INNO_VERSION_EXT(5, 6,  2, 0), 0 },
	{ "Inno Setup Setup Data (5.6.2) (u)", /* prerelease */ INNO_VERSION_EXT(5, 6,  2, 0), version::Unicode },
	{ "Inno Setup Setup Data (6.0.0) (u)",                  INNO_VERSION_EXT(6, 0,  0, 0), version::Unicode },
	{ "Inno Setup Setup Data (6.1.0) (u)",                  INNO_VERSION_EXT(6, 1,  0, 0), version::Unicode },
};

} // anonymous namespace

std::ostream & operator<<(std::ostream & os, const version & version) {
	
	os << version.a() << '.' << version.b() << '.' << version.c();
	if(version.d()) {
		os << '.' << version.d();
	}
	
	if(version.is_unicode()) {
		os << " (unicode)";
	}
	
	if(version.bits() != 32) {
		os << " (" << int(version.bits()) << "-bit)";
	}
	
	if(version.is_isx()) {
		os << " (isx)";
	}
	
	return os;
}

void version::load(std::istream & is) {
	
	static const char digits[] = "0123456789";
	
	BOOST_STATIC_ASSERT(sizeof(stored_legacy_version) <= sizeof(stored_version));
	
	stored_legacy_version legacy_version;
	is.read(legacy_version, std::streamsize(sizeof(legacy_version)));
	
	if(legacy_version[0] == 'i' && legacy_version[sizeof(legacy_version) - 1] == '\x1a') {
		
		for(size_t i = 0; i < size_t(boost::size(legacy_versions)); i++) {
			if(!memcmp(legacy_version, legacy_versions[i].name, sizeof(legacy_version))) {
				value = legacy_versions[i].version;
				variant = legacy_versions[i].variant;
				known = true;
				debug("known legacy version: \"" << versions[i].name << '"');
				return;
			}
		}
		
		debug("unknown legacy version: \""
		      << std::string(legacy_version, sizeof(legacy_version)) << '"');
		
		if(legacy_version[0] != 'i' || legacy_version[2] != '.' || legacy_version[4] != '.'
		   || legacy_version[7] != '-' || legacy_version[8] != '-') {
			throw version_error();
		}
		
		if(legacy_version[9] == '1' && legacy_version[10] == '6') {
			variant = Bits16;
		} else if(legacy_version[9] == '3' && legacy_version[10] == '2') {
			variant = 0;
		} else {
			throw version_error();
		}
		
		std::string version_str(legacy_version, sizeof(legacy_version));
		
		try {
			unsigned a = util::to_unsigned(version_str.data() + 1, 1);
			unsigned b = util::to_unsigned(version_str.data() + 3, 1);
			unsigned c = util::to_unsigned(version_str.data() + 5, 2);
			value = INNO_VERSION(a, b, c);
		} catch(const boost::bad_lexical_cast &) {
			throw version_error();
		}
		
		known = false;
		
		return;
	}
	
	stored_version version_string;
	BOOST_STATIC_ASSERT(sizeof(legacy_version) <= sizeof(version_string));
	memcpy(version_string, legacy_version, sizeof(legacy_version));
	is.read(version_string + sizeof(legacy_version),
	        std::streamsize(sizeof(version_string) - sizeof(legacy_version)));
	
	
	for(size_t i = 0; i < size_t(boost::size(versions)); i++) {
		if(versions[i].name[0] != '\0' && !memcmp(version_string, versions[i].name, sizeof(version_string))) {
			value = versions[i].version;
			variant = versions[i].variant;
			known = true;
			debug("known version: \"" << versions[i].name << '"');
			return;
		}
	}
	
	char * end = std::find(version_string, version_string + boost::size(version_string), '\0');
	std::string version_str(version_string, end);
	debug("unknown version: \"" << version_str << '"');
	if(!boost::contains(version_str, "Inno Setup")) {
		throw version_error();
	}
	
	value = 0;
	size_t bracket = version_str.find('(');
	for(; bracket != std::string::npos; bracket = version_str.find('(', bracket + 1)) {
		
		if(version_str.length() - bracket < 6) {
			continue;
		}
		
		try {
			
			size_t a_start = bracket + 1;
			size_t a_end = version_str.find_first_not_of(digits, a_start);
			if(a_end == std::string::npos || version_str[a_end] != '.') {
				continue;
			}
			unsigned a = util::to_unsigned(version_str.data() + a_start, a_end - a_start);
			
			size_t b_start = a_end + 1;
			size_t b_end = version_str.find_first_not_of(digits, b_start);
			if(b_end == std::string::npos || version_str[b_end] != '.') {
				continue;
			}
			unsigned b = util::to_unsigned(version_str.data() + b_start, b_end - b_start);
			
			size_t c_start = b_end + 1;
			size_t c_end = version_str.find_first_not_of(digits, c_start);
			if(c_end == std::string::npos) {
				continue;
			}
			unsigned c = util::to_unsigned(version_str.data() + c_start, c_end - c_start);
			
			size_t d_start = c_end;
			if(version_str[d_start] == 'a') {
				if(d_start + 1 >= version_str.length()) {
					continue;
				}
				d_start++;
			}
			
			unsigned d = 0;
			if(version_str[d_start] == '.') {
				d_start++;
				size_t d_end = version_str.find_first_not_of(digits, d_start);
				if(d_end != std::string::npos && d_end != d_start) {
					d = util::to_unsigned(version_str.data() + d_start, d_end - d_start);
				}
			}
			
			value = std::max(value, INNO_VERSION_EXT(a, b, c, d));
			
		} catch(const boost::bad_lexical_cast &) {
			continue;
		}
	}
	if(!value) {
		throw version_error();
	}
	
	variant = 0;
	if(boost::contains(version_str, "(u)") || boost::contains(version_str, "(U)")) {
		variant |= Unicode;
	}
	if(boost::contains(version_str, "My Inno Setup Extensions") || boost::contains(version_str, "with ISX")) {
		variant |= ISX;
	}
	
	known = false;
}

bool version::is_ambiguous() const {
	
	if(value == INNO_VERSION(1, 3, 21)) {
		// might be either 1.3.21 or 1.3.24
		return true;
	}
	
	if(value == INNO_VERSION(2, 0, 1)) {
		// might be either 2.0.1 or 2.0.2
		return true;
	}
	
	if(value == INNO_VERSION(3, 0, 3)) {
		// might be either 3.0.3 or 3.0.4
		return true;
	}
	
	if(value == INNO_VERSION(4, 2, 3)) {
		// might be either 4.2.3 or 4.2.4
		return true;
	}
	
	if(value == INNO_VERSION(5, 4, 2)) {
		// might be either 5.4.2 or 5.4.2.1
		return true;
	}
	
	if(value == INNO_VERSION(5, 5, 0)) {
		// might be either 5.5.0 or 5.5.0.1
		return true;
	}
	
	if(value == INNO_VERSION(5, 5, 7) || value == INNO_VERSION_EXT(5, 5, 7, 1)) {
		// might be either 5.5.7, an unknown modification of 5.5.7, or 5.6.0
		return true;
	}
	
	return false;
}

version_constant version::next() {
	
	const known_legacy_version * legacy_end = boost::end(legacy_versions);
	const known_legacy_version * legacy_result;
	legacy_result = std::upper_bound(boost::begin(legacy_versions), legacy_end, value);
	while(legacy_result != legacy_end && legacy_result->variant != variant) {
		legacy_result++;
	}
	if(legacy_result != legacy_end) {
		return legacy_result->version;
	}
	 
	const known_version * end = boost::end(versions);
	const known_version * result = std::upper_bound(boost::begin(versions), end, value);
	while(result != end && result->variant != variant) {
		result++;
	}
	if(result != end) {
		return result->version;
	}
	
	return 0;
}

} // namespace setup

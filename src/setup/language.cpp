/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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

#include "setup/language.hpp"

#include <algorithm>

#include "boost/range/begin.hpp"
#include "boost/range/end.hpp"

#include "setup/version.hpp"
#include "util/load.hpp"

namespace setup {

namespace {

struct windows_language {
	
	boost::uint16_t language_id;
	boost::uint16_t codepage;
	
	bool operator<(boost::uint32_t language) const {
		return language_id < language;
	}
	
};

/*
 * Sorted list of Windows language IDs with their default ANSI codepages.
 * This list omits unicode-only languages and languages using the default Windows-1252 codepage.
 */
windows_language languages[] = {
	{ 0x0401, 1256 },
	{ 0x0402, 1251 },
	{ 0x0404, 950 },
	{ 0x0405, 1250 },
	{ 0x0408, 1253 },
	{ 0x040d, 1255 },
	{ 0x040e, 1250 },
	{ 0x0411, 932 },
	{ 0x0412, 949 },
	{ 0x0415, 1250 },
	{ 0x0418, 1250 },
	{ 0x0419, 1251 },
	{ 0x041a, 1250 },
	{ 0x041b, 1250 },
	{ 0x041c, 1250 },
	{ 0x041e, 874 },
	{ 0x041f, 1254 },
	{ 0x0420, 1256 },
	{ 0x0422, 1251 },
	{ 0x0423, 1251 },
	{ 0x0424, 1250 },
	{ 0x0425, 1257 },
	{ 0x0426, 1257 },
	{ 0x0427, 1257 },
	{ 0x0429, 1256 },
	{ 0x042a, 1258 },
	{ 0x042c, 1254 },
	{ 0x042f, 1251 },
	{ 0x043f, 1251 },
	{ 0x0440, 1251 },
	{ 0x0443, 1254 },
	{ 0x0444, 1251 },
	{ 0x0450, 1251 },
	{ 0x0492, 28604 },
	{ 0x0801, 1256 },
	{ 0x0804, 936 },
	{ 0x081a, 1250 },
	{ 0x082c, 1251 },
	{ 0x0843, 1251 },
	{ 0x0c01, 1256 },
	{ 0x0c04, 950 },
	{ 0x0c1a, 1251 },
	{ 0x1001, 1256 },
	{ 0x1004, 936 },
	{ 0x1401, 1256 },
	{ 0x1404, 950 },
	{ 0x1801, 1256 },
	{ 0x1c01, 1256 },
	{ 0x2001, 1256 },
	{ 0x2401, 1256 },
	{ 0x2801, 1256 },
	{ 0x2c01, 1256 },
	{ 0x3001, 1256 },
	{ 0x3401, 1256 },
	{ 0x3801, 1256 },
	{ 0x3c01, 1256 },
	{ 0x4001, 1256 },
};

util::codepage_id default_codepage_for_language(boost::uint32_t language) {
	
	windows_language * entry = std::lower_bound(boost::begin(languages), boost::end(languages), language);
	if(entry != boost::end(languages) && entry->language_id == language) {
		return entry->codepage;
	}
	
	return util::cp_windows1252;
}

} // anonymous namespace

void language_entry::load(std::istream & is, const version & version) {
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		is >> util::encoded_string(name, version.codepage());
	} else {
		name = "default";
	}
	
	is >> util::binary_string(language_name);
	
	if(version == INNO_VERSION_EXT(5, 5,  7, 1)) {
		util::load<boost::uint32_t>(is); // always 0?
	}
	
	is >> util::binary_string(dialog_font);
	is >> util::binary_string(title_font);
	is >> util::binary_string(welcome_font);
	is >> util::binary_string(copyright_font);
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		is >> util::binary_string(data);
	}
	
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> util::binary_string(license_text);
		is >> util::binary_string(info_before);
		is >> util::binary_string(info_after);
	} else {
		license_text.clear(), info_before.clear(), info_after.clear();
	}
	
	language_id = util::load<boost::uint32_t>(is);
	
	if(version < INNO_VERSION(4, 2, 2)) {
		codepage = default_codepage_for_language(language_id);
	} else if(version < INNO_VERSION(5, 3, 0) || !version.is_unicode()) {
		codepage = util::load<boost::uint32_t>(is);
		if(!codepage) {
			codepage = version.codepage();
		}
	} else {
		codepage = util::cp_utf16le;
	}
	
	if(version >= INNO_VERSION(4, 2, 2)) {
		util::to_utf8(language_name, util::cp_utf16le);
	} else {
		util::to_utf8(language_name, codepage);
	}
	
	dialog_font_size = util::load<boost::uint32_t>(is);
	
	if(version < INNO_VERSION(4, 1, 0)) {
		dialog_font_standard_height = util::load<boost::uint32_t>(is);
	} else {
		dialog_font_standard_height = 0;
	}
	
	title_font_size = util::load<boost::uint32_t>(is);
	welcome_font_size = util::load<boost::uint32_t>(is);
	copyright_font_size = util::load<boost::uint32_t>(is);
	
	if(version == INNO_VERSION_EXT(5, 5,  7, 1)) {
		util::load<boost::uint32_t>(is); // always 8 or 9?
	}
	
	if(version >= INNO_VERSION(5, 2, 3)) {
		right_to_left = util::load_bool(is);
	} else {
		right_to_left = false;
	}
	
}

} // namespace setup

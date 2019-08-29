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

#include "setup/language.hpp"

#include <algorithm>

#include "boost/range/begin.hpp"
#include "boost/range/end.hpp"

#include "setup/info.hpp"
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
 * This list omits Unicode-only languages and languages using the default Windows-1252 codepage.
 */
const windows_language languages[] = {
	{ 0x0401, util::cp_windows1256 },
	{ 0x0402, util::cp_windows1251 },
	{ 0x0404, util::cp_big5 },
	{ 0x0405, util::cp_windows1250 },
	{ 0x0408, util::cp_windows1253 },
	{ 0x040d, util::cp_windows1255 },
	{ 0x040e, util::cp_windows1250 },
	{ 0x0411, util::cp_shift_jis },
	{ 0x0412, util::cp_uhc },
	{ 0x0415, util::cp_windows1250 },
	{ 0x0418, util::cp_windows1250 },
	{ 0x0419, util::cp_windows1251 },
	{ 0x041a, util::cp_windows1250 },
	{ 0x041b, util::cp_windows1250 },
	{ 0x041c, util::cp_windows1250 },
	{ 0x041e, util::cp_windows874 },
	{ 0x041f, util::cp_windows1254 },
	{ 0x0420, util::cp_windows1256 },
	{ 0x0422, util::cp_windows1251 },
	{ 0x0423, util::cp_windows1251 },
	{ 0x0424, util::cp_windows1250 },
	{ 0x0425, util::cp_windows1257 },
	{ 0x0426, util::cp_windows1257 },
	{ 0x0427, util::cp_windows1257 },
	{ 0x0429, util::cp_windows1256 },
	{ 0x042a, util::cp_windows1258 },
	{ 0x042c, util::cp_windows1254 },
	{ 0x042f, util::cp_windows1251 },
	{ 0x043f, util::cp_windows1251 },
	{ 0x0440, util::cp_windows1251 },
	{ 0x0443, util::cp_windows1254 },
	{ 0x0444, util::cp_windows1251 },
	{ 0x0450, util::cp_windows1251 },
	{ 0x0492, util::cp_iso_8859_14 },
	{ 0x0801, util::cp_windows1256 },
	{ 0x0804, util::cp_gbk },
	{ 0x081a, util::cp_windows1250 },
	{ 0x082c, util::cp_windows1251 },
	{ 0x0843, util::cp_windows1251 },
	{ 0x0c01, util::cp_windows1256 },
	{ 0x0c04, util::cp_big5 },
	{ 0x0c1a, util::cp_windows1251 },
	{ 0x1001, util::cp_windows1256 },
	{ 0x1004, util::cp_gbk },
	{ 0x1401, util::cp_windows1256 },
	{ 0x1404, util::cp_big5 },
	{ 0x1801, util::cp_windows1256 },
	{ 0x1c01, util::cp_windows1256 },
	{ 0x2001, util::cp_windows1256 },
	{ 0x2401, util::cp_windows1256 },
	{ 0x2801, util::cp_windows1256 },
	{ 0x2c01, util::cp_windows1256 },
	{ 0x3001, util::cp_windows1256 },
	{ 0x3401, util::cp_windows1256 },
	{ 0x3801, util::cp_windows1256 },
	{ 0x3c01, util::cp_windows1256 },
	{ 0x4001, util::cp_windows1256 },
};

util::codepage_id default_codepage_for_language(boost::uint32_t language) {
	
	const windows_language * entry = std::lower_bound(boost::begin(languages), boost::end(languages), language);
	if(entry != boost::end(languages) && entry->language_id == language) {
		return entry->codepage;
	}
	
	return util::cp_windows1252;
}

} // anonymous namespace

void language_entry::load(std::istream & is, const info & i) {
	
	if(i.version >= INNO_VERSION(4, 0, 0)) {
		is >> util::binary_string(name);
	}
	
	is >> util::binary_string(language_name);
	
	if(i.version == INNO_VERSION_EXT(5, 5, 7, 1)) {
		util::binary_string::skip(is);
	}
	
	is >> util::binary_string(dialog_font);
	is >> util::binary_string(title_font);
	is >> util::binary_string(welcome_font);
	is >> util::binary_string(copyright_font);
	
	if(i.version >= INNO_VERSION(4, 0, 0)) {
		is >> util::binary_string(data);
	}
	
	if(i.version >= INNO_VERSION(4, 0, 1)) {
		is >> util::binary_string(license_text);
		is >> util::binary_string(info_before);
		is >> util::binary_string(info_after);
	} else {
		license_text.clear(), info_before.clear(), info_after.clear();
	}
	
	language_id = util::load<boost::uint32_t>(is);
	
	if(i.version < INNO_VERSION(4, 2, 2)) {
		codepage = default_codepage_for_language(language_id);
	} else if(!i.version.is_unicode()) {
		codepage = util::load<boost::uint32_t>(is);
		if(!codepage) {
			codepage = util::cp_windows1252;
		}
	} else {
		if(i.version < INNO_VERSION(5, 3, 0)) {
			(void)util::load<boost::uint32_t>(is);
		}
		codepage = util::cp_utf16le;
	}
	
	if(i.version >= INNO_VERSION(4, 2, 2)) {
		util::to_utf8(language_name, util::cp_utf16le);
	} else {
		util::to_utf8(language_name, codepage);
	}
	
	dialog_font_size = util::load<boost::uint32_t>(is);
	
	if(i.version < INNO_VERSION(4, 1, 0)) {
		dialog_font_standard_height = util::load<boost::uint32_t>(is);
	} else {
		dialog_font_standard_height = 0;
	}
	
	title_font_size = util::load<boost::uint32_t>(is);
	welcome_font_size = util::load<boost::uint32_t>(is);
	copyright_font_size = util::load<boost::uint32_t>(is);
	
	if(i.version == INNO_VERSION_EXT(5, 5, 7, 1)) {
		util::load<boost::uint32_t>(is); // always 8 or 9?
	}
	
	if(i.version >= INNO_VERSION(5, 2, 3)) {
		right_to_left = util::load_bool(is);
	} else {
		right_to_left = false;
	}
	
}

void language_entry::decode(util::codepage_id cp) {
	
	util::to_utf8(name, cp);
	if(name.empty()) {
		name = "default";
	}
	
}

} // namespace setup

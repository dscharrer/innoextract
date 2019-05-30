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

#include "setup/version.hpp"
#include "util/load.hpp"

namespace setup {

void language_entry::load(std::istream & is, const version & version) {
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		is >> util::encoded_string(name, version.codepage());
	} else {
		name = "default";
	}
	
	is >> util::encoded_string(language_name, (version >= INNO_VERSION(4, 2, 2)) ? 1200u : 1252u);
	
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
	
	if(version >= INNO_VERSION(4, 2, 2) && (version < INNO_VERSION(5, 3, 0) || !version.is_unicode())) {
		codepage = util::load<boost::uint32_t>(is);
	} else {
		codepage = 0;
	}
	if(!codepage) {
		codepage = version.codepage();
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

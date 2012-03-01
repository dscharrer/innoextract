/*
 * Copyright (C) 2011 Daniel Scharrer
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
		is >> encoded_string(name, version.codepage());
	} else {
		name = "default";
	}
	
	is >> encoded_string(language_name, (version >= INNO_VERSION(4, 2, 2)) ? 1200u : 1252u);
	
	is >> encoded_string(dialog_font, version.codepage());
	is >> encoded_string(title_font, version.codepage());
	is >> encoded_string(welcome_font, version.codepage());
	is >> encoded_string(copyright_font, version.codepage());
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		is >> binary_string(data);
	}
	
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> ansi_string(license_text);
		is >> ansi_string(info_before);
		is >> ansi_string(info_after);
	} else {
		license_text.clear(), info_before.clear(), info_after.clear();
	}
	
	language_id = load_number<uint32_t>(is);
	
	if(version >= INNO_VERSION(4, 2, 2) && (version < INNO_VERSION(5, 3, 0) || !version.unicode)) {
		codepage = load_number<uint32_t>(is);
	} else {
		codepage = 0;
	}
	if(!codepage) {
		codepage = version.codepage();
	}
	
	dialog_font_size = load_number<uint32_t>(is);
	
	if(version < INNO_VERSION(4, 1, 0)) {
		dialog_font_standard_height = load_number<uint32_t>(is);
	} else {
		dialog_font_standard_height = 0;
	}
	
	title_font_size = load_number<uint32_t>(is);
	welcome_font_size = load_number<uint32_t>(is);
	copyright_font_size = load_number<uint32_t>(is);
	
	if(version >= INNO_VERSION(5, 2, 3)) {
		right_to_left = ::load<uint8_t>(is);
	} else {
		right_to_left = false;
	}
	
}

} // namespace setup

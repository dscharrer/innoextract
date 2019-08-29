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

#include "setup/item.hpp"

#include "setup/info.hpp"
#include "setup/version.hpp"
#include "util/load.hpp"

namespace setup {

void item::load_condition_data(std::istream & is, const info & i) {
	
	if(i.version >= INNO_VERSION(2, 0, 0) || (i.version.is_isx() && i.version >= INNO_VERSION(1, 3, 8))) {
		is >> util::encoded_string(components, i.codepage);
	} else {
		components.clear();
	}
	if(i.version >= INNO_VERSION(2, 0, 0) || (i.version.is_isx() && i.version >= INNO_VERSION(1, 3, 17))) {
		is >> util::encoded_string(tasks, i.codepage);
	} else {
		tasks.clear();
	}
	if(i.version >= INNO_VERSION(4, 0, 1)) {
		is >> util::encoded_string(languages, i.codepage);
	} else {
		languages.clear();
	}
	if(i.version >= INNO_VERSION(4, 0, 0) || (i.version.is_isx() && i.version >= INNO_VERSION(1, 3, 24))) {
		is >> util::encoded_string(check, i.codepage);
	} else {
		check.clear();
	}
	
	if(i.version >= INNO_VERSION(4, 1, 0)) {
		is >> util::encoded_string(after_install, i.codepage);
		is >> util::encoded_string(before_install, i.codepage);
	} else {
		after_install.clear(), before_install.clear();
	}
	
}

} // namespace setup

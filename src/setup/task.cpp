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

#include "setup/task.hpp"

#include <boost/cstdint.hpp>

#include "setup/info.hpp"
#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

void task_entry::load(std::istream & is, const info & i) {
	
	is >> util::encoded_string(name, i.codepage);
	is >> util::encoded_string(description, i.codepage);
	is >> util::encoded_string(group_description, i.codepage);
	is >> util::encoded_string(components, i.codepage);
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
	if(i.version >= INNO_VERSION(4, 0, 0) || (i.version.is_isx() && i.version >= INNO_VERSION(3, 0, 3))) {
		level = util::load<boost::int32_t>(is);
	} else {
		level = 0;
	}
	if(i.version >= INNO_VERSION(4, 0, 0) || (i.version.is_isx() && i.version >= INNO_VERSION(3, 0, 4))) {
		used = util::load_bool(is);
	} else {
		used = true;
	}
	
	winver.load(is, i.version);
	
	stored_flag_reader<flags> flagreader(is);
	
	flagreader.add(Exclusive);
	flagreader.add(Unchecked);
	if(i.version >= INNO_VERSION(2, 0, 5)) {
		flagreader.add(Restart);
	}
	if(i.version >= INNO_VERSION(2, 0, 6)) {
		flagreader.add(CheckedOnce);
	}
	if(i.version >= INNO_VERSION(4, 2, 3)) {
		flagreader.add(DontInheritCheck);
	}
	
	options = flagreader;
}

} // namespace setup

NAMES(setup::task_entry::flags, "Setup Task Option",
	"exclusive",
	"unchecked",
	"restart",
	"checked once",
	"don't inherit check",
)

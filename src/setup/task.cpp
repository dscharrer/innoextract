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

#include "setup/task.hpp"

#include <boost/cstdint.hpp>

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

void task_entry::load(std::istream & is, const version & version) {
	
	is >> encoded_string(name, version.codepage());
	is >> encoded_string(description, version.codepage());
	is >> encoded_string(group_description, version.codepage());
	is >> encoded_string(components, version.codepage());
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> encoded_string(languages, version.codepage());
	} else {
		languages.clear();
	}
	if(version >= INNO_VERSION_EXT(3, 0, 6, 1)) {
		is >> encoded_string(check, version.codepage());
		level = load_number<boost::int32_t>(is);
		used = load_bool(is);
	} else {
		check.clear(), level = 0, used = true;
	}
	
	winver.load(is, version);
	
	stored_flag_reader<flags> flagreader(is);
	
	flagreader.add(Exclusive);
	flagreader.add(Unchecked);
	if(version >= INNO_VERSION(2, 0, 5)) {
		flagreader.add(Restart);
	}
	if(version >= INNO_VERSION(2, 0, 6)) {
		flagreader.add(CheckedOnce);
	}
	if(version >= INNO_VERSION(4, 2, 3)) {
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

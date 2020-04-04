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

#include "setup/delete.hpp"

#include "setup/info.hpp"
#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

STORED_ENUM_MAP(delete_target_type_map, delete_entry::Files,
	delete_entry::Files,
	delete_entry::FilesAndSubdirs,
	delete_entry::DirIfEmpty,
);

} // anonymous namespace

void delete_entry::load(std::istream & is, const info & i) {
	
	if(i.version < INNO_VERSION(1, 3, 0)) {
		(void)util::load<boost::uint32_t>(is); // uncompressed size of the entry
	}
	
	is >> util::encoded_string(name, i.codepage, i.header.lead_bytes);
	
	load_condition_data(is, i);
	
	load_version_data(is, i.version);
	
	type = stored_enum<delete_target_type_map>(is).get();
}

} // namespace setup

NAMES(setup::delete_entry::target_type, "Delete Type",
	"files",
	"files and subdirs",
	"dir if empty",
)

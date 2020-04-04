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

#include "setup/directory.hpp"

#include "setup/info.hpp"
#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

STORED_FLAGS_MAP(stored_inno_directory_options_0,
	directory_entry::NeverUninstall,
	directory_entry::DeleteAfterInstall,
	directory_entry::AlwaysUninstall,
);

// starting with version 5.2.0
STORED_FLAGS_MAP(stored_inno_directory_options_1,
	directory_entry::NeverUninstall,
	directory_entry::DeleteAfterInstall,
	directory_entry::AlwaysUninstall,
	directory_entry::SetNtfsCompression,
	directory_entry::UnsetNtfsCompression,
);

} // anonymous namespace

void directory_entry::load(std::istream & is, const info & i) {
	
	if(i.version < INNO_VERSION(1, 3, 0)) {
		(void)util::load<boost::uint32_t>(is); // uncompressed size of the entry
	}
	
	is >> util::encoded_string(name, i.codepage, i.header.lead_bytes);
	
	load_condition_data(is, i);
	
	if(i.version >= INNO_VERSION(4, 0, 11) && i.version < INNO_VERSION(4, 1, 0)) {
		is >> util::binary_string(permissions);
	} else {
		permissions.clear();
	}
	
	if(i.version >= INNO_VERSION(2, 0, 11)) {
		attributes = util::load<boost::uint32_t>(is);
	} else {
		attributes = 0;
	}
	
	load_version_data(is, i.version);
	
	if(i.version >= INNO_VERSION(4, 1, 0)) {
		permission = util::load<boost::int16_t>(is);
	} else {
		permission = boost::int16_t(-1);
	}
	
	if(i.version >= INNO_VERSION(5, 2, 0)) {
		options = stored_flags<stored_inno_directory_options_1>(is).get();
	} else if(i.version.bits() != 16) {
		options = stored_flags<stored_inno_directory_options_0>(is).get();
	} else {
		options = stored_flags<stored_inno_directory_options_0, 16>(is).get();
	}
	
}

} // namespace setup

NAMES(setup::directory_entry::flags, "Directory Option",
	"never uninstall",
	"delete after install",
	"always uninstall",
	"set NTFS compression",
	"unset NTFS compression",
)

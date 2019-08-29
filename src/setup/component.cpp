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

#include "setup/component.hpp"

#include "setup/info.hpp"
#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

STORED_FLAGS_MAP(stored_component_flags_0,
	component_entry::Fixed,
	component_entry::Restart,
	component_entry::DisableNoUninstallWarning,
);

// starting with version 3.0.8
STORED_FLAGS_MAP(stored_component_flags_1,
	component_entry::Fixed,
	component_entry::Restart,
	component_entry::DisableNoUninstallWarning,
	component_entry::Exclusive,
);

// starting with version 4.2.3
STORED_FLAGS_MAP(stored_component_flags_2,
	component_entry::Fixed,
	component_entry::Restart,
	component_entry::DisableNoUninstallWarning,
	component_entry::Exclusive,
	component_entry::DontInheritCheck,
);

} // anonymous namespace

void component_entry::load(std::istream & is, const info & i) {
	
	is >> util::encoded_string(name, i.codepage);
	is >> util::encoded_string(description, i.codepage);
	is >> util::encoded_string(types, i.codepage);
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
	if(i.version >= INNO_VERSION(4, 0, 0)) {
		extra_disk_pace_required = util::load<boost::uint64_t>(is);
	} else {
		extra_disk_pace_required = util::load<boost::uint32_t>(is);
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
	
	if(i.version >= INNO_VERSION(4, 2, 3)) {
		options = stored_flags<stored_component_flags_2>(is).get();
	} else if(i.version >= INNO_VERSION(3, 0, 8) ||
		        (i.version.is_isx() && i.version >= INNO_VERSION_EXT(3, 0, 6, 1))) {
		options = stored_flags<stored_component_flags_1>(is).get();
	} else {
		options = stored_flags<stored_component_flags_0>(is).get();
	}
	
	if(i.version >= INNO_VERSION(4, 0, 0)) {
		size = util::load<boost::uint64_t>(is);
	} else if(i.version >= INNO_VERSION(2, 0, 0) ||
		        (i.version.is_isx() && i.version >= INNO_VERSION(1, 3, 24))) {
		size = util::load<boost::uint32_t>(is);
	}
}

} // namespace setup

NAMES(setup::component_entry::flags, "Setup Component Option",
	"fixed",
	"restart",
	"disable no uninstall warning",
	"exclusive",
	"don't inherit check",
)

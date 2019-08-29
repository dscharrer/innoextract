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

#include "setup/type.hpp"

#include "setup/info.hpp"
#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

FLAGS(type_flags,
	CustomSetupType
);

STORED_FLAGS_MAP(stored_type_flags,
	CustomSetupType,
);

STORED_ENUM_MAP(stored_setup_type, type_entry::User,
	type_entry::User,
	type_entry::DefaultFull,
	type_entry::DefaultCompact,
	type_entry::DefaultCustom,
);

} // anonymous namespace

} // namespace setup

NAMED_FLAGS(setup::type_flags)

namespace setup {

void type_entry::load(std::istream & is, const info & i) {
	
	USE_FLAG_NAMES(setup::type_flags)
	
	is >> util::encoded_string(name, i.codepage);
	is >> util::encoded_string(description, i.codepage);
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
	
	winver.load(is, i.version);
	
	type_flags options = stored_flags<stored_type_flags>(is).get();
	custom_type = ((options & CustomSetupType) != 0);
	
	if(i.version >= INNO_VERSION(4, 0, 3)) {
		type = stored_enum<stored_setup_type>(is).get();
	} else {
		type = User;
	}
	
	if(i.version >= INNO_VERSION(4, 0, 0)) {
		size = util::load<boost::uint64_t>(is);
	} else {
		size = util::load<boost::uint32_t>(is);
	}
}

} // namespace setup

NAMES(setup::type_flags, "Setyp Type Option",
	"is custom",
)

NAMES(setup::type_entry::setup_type, "Setyp Type",
	"user",
	"default full",
	"default compact",
	"default custom",
)

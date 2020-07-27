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

#include "setup/run.hpp"

#include <boost/cstdint.hpp>

#include "setup/info.hpp"
#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

STORED_ENUM_MAP(stored_run_wait_condition, run_entry::WaitUntilTerminated,
	run_entry::WaitUntilTerminated,
	run_entry::NoWait,
	run_entry::WaitUntilIdle,
);

} // anonymous namespace

void run_entry::load(std::istream & is, const info & i) {
	
	if(i.version < INNO_VERSION(1, 3, 0)) {
		(void)util::load<boost::uint32_t>(is); // uncompressed size of the entry
	}
	
	is >> util::encoded_string(name, i.codepage, i.header.lead_bytes);
	is >> util::encoded_string(parameters, i.codepage, i.header.lead_bytes);
	is >> util::encoded_string(working_dir, i.codepage, i.header.lead_bytes);
	if(i.version >= INNO_VERSION(1, 3, 9)) {
		is >> util::encoded_string(run_once_id, i.codepage);
	} else {
		run_once_id.clear();
	}
	if(i.version >= INNO_VERSION(2, 0, 2)) {
		is >> util::encoded_string(status_message, i.codepage);
	} else {
		status_message.clear();
	}
	if(i.version >= INNO_VERSION(5, 1, 13)) {
		is >> util::encoded_string(verb, i.codepage);
	} else {
		verb.clear();
	}
	if(i.version >= INNO_VERSION(2, 0, 0) || i.version.is_isx()) {
		is >> util::encoded_string(description, i.codepage);
	}
	
	load_condition_data(is, i);
	
	load_version_data(is, i.version);
	
	if(i.version >= INNO_VERSION(1, 3, 24)) {
		show_command = util::load<boost::int32_t>(is);
	} else {
		show_command = 0;
	}
	
	wait = stored_enum<stored_run_wait_condition>(is).get();
	
	stored_flag_reader<flags> flagreader(is, i.version.bits());
	
	if(i.version >= INNO_VERSION(1, 2, 3)) {
		flagreader.add(ShellExec);
	}
	if(i.version >= INNO_VERSION(1, 3, 9) || (i.version.is_isx() && i.version >= INNO_VERSION(1, 3, 8))) {
		flagreader.add(SkipIfDoesntExist);
	}
	if(i.version >= INNO_VERSION(2, 0, 0)) {
		flagreader.add(PostInstall);
		flagreader.add(Unchecked);
		flagreader.add(SkipIfSilent);
		flagreader.add(SkipIfNotSilent);
	}
	if(i.version >= INNO_VERSION(2, 0, 8)) {
		flagreader.add(HideWizard);
	}
	if(i.version >= INNO_VERSION(5, 1, 10)) {
		flagreader.add(Bits32);
		flagreader.add(Bits64);
	}
	if(i.version >= INNO_VERSION(5, 2, 0)) {
		flagreader.add(RunAsOriginalUser);
	}
	if(i.version >= INNO_VERSION(6, 1, 0)) {
		flagreader.add(DontLogParameters);
	}
	
	options = flagreader;
}

} // namespace setup

NAMES(setup::run_entry::flags, "Run Option",
	"shell exec",
	"skip if doesn't exist",
	"post install",
	"unchecked",
	"skip if silent",
	"skip if not silent",
	"hide wizard",
	"32 bit",
	"64 bit",
	"run as original user",
)

NAMES(setup::run_entry::wait_condition, "Run Wait Type",
	"wait until terminated",
	"no wait",
	"wait until idle",
)

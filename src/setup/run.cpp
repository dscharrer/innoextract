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

#include "setup/run.hpp"

#include <boost/cstdint.hpp>

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

void run_entry::load(std::istream & is, const version & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		(void)util::load<boost::uint32_t>(is); // uncompressed size of the entry
	}
	
	is >> util::encoded_string(name, version.codepage());
	is >> util::encoded_string(parameters, version.codepage());
	is >> util::encoded_string(working_dir, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> util::encoded_string(run_once_id, version.codepage());
	} else {
		run_once_id.clear();
	}
	if(version >= INNO_VERSION(2, 0, 2)) {
		is >> util::encoded_string(status_message, version.codepage());
	} else {
		status_message.clear();
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		is >> util::encoded_string(verb, version.codepage());
	} else {
		verb.clear();
	}
	if(version >= INNO_VERSION(2, 0, 0)) {
		is >> util::encoded_string(description, version.codepage());
	}
	
	load_condition_data(is, version);
	
	load_version_data(is, version);
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		show_command = util::load<boost::int32_t>(is);
	} else {
		show_command = 0;
	}
	
	wait = stored_enum<stored_run_wait_condition>(is).get();
	
	stored_flag_reader<flags> flagreader(is, version.bits());
	
	flagreader.add(ShellExec);
	if(version >= INNO_VERSION(1, 3, 21)) {
		flagreader.add(SkipIfDoesntExist);
	}
	if(version >= INNO_VERSION(2, 0, 0)) {
		flagreader.add(PostInstall);
		flagreader.add(Unchecked);
		flagreader.add(SkipIfSilent);
		flagreader.add(SkipIfNotSilent);
	}
	if(version >= INNO_VERSION(2, 0, 8)) {
		flagreader.add(HideWizard);
	}
	if(version >= INNO_VERSION(5, 1, 10)) {
		flagreader.add(Bits32);
		flagreader.add(Bits64);
	}
	if(version >= INNO_VERSION(5, 2, 0)) {
		flagreader.add(RunAsOriginalUser);
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

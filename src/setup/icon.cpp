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

#include "setup/icon.hpp"

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

STORED_ENUM_MAP(stored_close_setting, icon_entry::NoSetting,
	icon_entry::NoSetting,
	icon_entry::CloseOnExit,
	icon_entry::DontCloseOnExit,
);

} // anonymous namespace

void icon_entry::load(std::istream & is, const version & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		(void)util::load<boost::uint32_t>(is); // uncompressed size of the entry
	}
	
	is >> util::encoded_string(name, version.codepage());
	is >> util::encoded_string(filename, version.codepage());
	is >> util::encoded_string(parameters, version.codepage());
	is >> util::encoded_string(working_dir, version.codepage());
	is >> util::encoded_string(icon_file, version.codepage());
	is >> util::encoded_string(comment, version.codepage());
	
	load_condition_data(is, version);
	
	if(version >= INNO_VERSION(5, 3, 5)) {
		is >> util::encoded_string(app_user_model_id, version.codepage());
	} else {
		app_user_model_id.clear();
	}
	
	load_version_data(is, version);
	
	icon_index = util::load<boost::int32_t>(is, version.bits);
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		show_command = util::load<boost::int32_t>(is);
		close_on_exit = stored_enum<stored_close_setting>(is).get();
	} else {
		show_command = 1, close_on_exit = NoSetting;
	}
	
	if(version >= INNO_VERSION(2, 0, 7)) {
		hotkey = util::load<boost::uint16_t>(is);
	} else {
		hotkey = 0;
	}
	
	stored_flag_reader<flags> flagreader(is, version.bits);
	
	flagreader.add(NeverUninstall);
	if(version >= INNO_VERSION(1, 3, 21) && version < INNO_VERSION(1, 3, 26)) {
		flagreader.add(RunMinimized);
	}
	flagreader.add(CreateOnlyIfFileExists);
	if(version.bits != 16) {
		flagreader.add(UseAppPaths);
	}
	if(version >= INNO_VERSION(5, 0, 3)) {
		flagreader.add(FolderShortcut);
	}
	if(version >= INNO_VERSION(5, 4, 2)) {
		flagreader.add(ExcludeFromShowInNewInstall);
	}
	if(version >= INNO_VERSION(5, 5, 0)) {
		flagreader.add(PreventPinning);
	}
	
	options = flagreader;
}

} // namespace setup

NAMES(setup::icon_entry::flags, "Icon Option",
	"never uninstall",
	"create only if file exists",
	"use app paths",
	"folder shortcut",
	"exclude from show in new install",
	"prevent pinning",
	"run minimized",
)

NAMES(setup::icon_entry::close_setting, "Close on Exit",
	"no setting",
	"close on exit",
	"don't close on exit",
)

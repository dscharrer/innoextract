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

#include "setup/icon.hpp"

#include "setup/info.hpp"
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

void icon_entry::load(std::istream & is, const info & i) {
	
	if(i.version < INNO_VERSION(1, 3, 0)) {
		(void)util::load<boost::uint32_t>(is); // uncompressed size of the entry
	}
	
	is >> util::encoded_string(name, i.codepage, i.header.lead_bytes);
	is >> util::encoded_string(filename, i.codepage, i.header.lead_bytes);
	is >> util::encoded_string(parameters, i.codepage, i.header.lead_bytes);
	is >> util::encoded_string(working_dir, i.codepage, i.header.lead_bytes);
	is >> util::encoded_string(icon_file, i.codepage, i.header.lead_bytes);
	is >> util::encoded_string(comment, i.codepage);
	
	load_condition_data(is, i);
	
	if(i.version >= INNO_VERSION(5, 3, 5)) {
		is >> util::encoded_string(app_user_model_id, i.codepage);
	} else {
		app_user_model_id.clear();
	}
	
	if(i.version >= INNO_VERSION(6, 1, 0)) {
		const size_t guid_size = 16;
		app_user_model_toast_activator_clsid.resize(guid_size);
		is.read(&app_user_model_toast_activator_clsid[0], std::streamsize(guid_size));
	} else {
		app_user_model_toast_activator_clsid.clear();
	}
	
	load_version_data(is, i.version);
	
	icon_index = util::load<boost::int32_t>(is, i.version.bits());
	
	if(i.version >= INNO_VERSION(1, 3, 24)) {
		show_command = util::load<boost::int32_t>(is);
	} else {
		show_command = 1;
	}
	if(i.version >= INNO_VERSION(1, 3, 15)) {
		close_on_exit = stored_enum<stored_close_setting>(is).get();
	} else {
		close_on_exit = NoSetting;
	}
	
	if(i.version >= INNO_VERSION(2, 0, 7)) {
		hotkey = util::load<boost::uint16_t>(is);
	} else {
		hotkey = 0;
	}
	
	stored_flag_reader<flags> flagreader(is, i.version.bits());
	
	flagreader.add(NeverUninstall);
	if(i.version < INNO_VERSION(1, 3, 26)) {
		flagreader.add(RunMinimized);
	}
	flagreader.add(CreateOnlyIfFileExists);
	if(i.version.bits() != 16) {
		flagreader.add(UseAppPaths);
	}
	if(i.version >= INNO_VERSION(5, 0, 3)) {
		flagreader.add(FolderShortcut);
	}
	if(i.version >= INNO_VERSION(5, 4, 2)) {
		flagreader.add(ExcludeFromShowInNewInstall);
	}
	if(i.version >= INNO_VERSION(5, 5, 0)) {
		flagreader.add(PreventPinning);
	}
	if(i.version >= INNO_VERSION(6, 1, 0)) {
		flagreader.add(HasAppUserModelToastActivatorCLSID);
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


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
		::load<uint32_t>(is); // uncompressed size of the icon entry structure
	}
	
	is >> encoded_string(name, version.codepage());
	is >> encoded_string(filename, version.codepage());
	is >> encoded_string(parameters, version.codepage());
	is >> encoded_string(working_dir, version.codepage());
	is >> encoded_string(icon_file, version.codepage());
	is >> encoded_string(comment, version.codepage());
	
	load_condition_data(is, version);
	
	if(version >= INNO_VERSION(5, 3, 5)) {
		is >> encoded_string(app_user_model_id, version.codepage());
	} else {
		app_user_model_id.clear();
	}
	
	load_version_data(is, version);
	
	icon_index = load_number<int32_t>(is, version.bits);
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		show_command = load_number<int32_t>(is);
		close_on_exit = stored_enum<stored_close_setting>(is).get();
	} else {
		show_command = 1, close_on_exit = NoSetting;
	}
	
	if(version >= INNO_VERSION(2, 0, 7)) {
		hotkey = load_number<uint16_t>(is);
	} else {
		hotkey = 0;
	}
	
	stored_flag_reader<flags> flags(is);
	
	flags.add(NeverUninstall);
	if(version >= INNO_VERSION(1, 3, 21)) {
		flags.add(RunMinimized);
	}
	flags.add(CreateOnlyIfFileExists);
	if(version.bits != 16) {
		flags.add(UseAppPaths);
	}
	if(version >= INNO_VERSION(5, 0, 3)) {
		flags.add(FolderShortcut);
	}
	if(version >= INNO_VERSION(5, 4, 2)) {
		flags.add(ExcludeFromShowInNewInstall);
	}
	
	options = flags;
}

} // namespace setup

NAMES(setup::icon_entry::flags, "Icon Option",
	"never uninstall",
	"create only if file exists",
	"use app paths",
	"folder shortcut",
	"exclude from show in new install",
	"run minimized",
)

NAMES(setup::icon_entry::close_setting, "Close on Exit",
	"no setting",
	"close on exit",
	"don't close on exit",
)

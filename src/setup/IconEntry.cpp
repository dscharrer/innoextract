
#include "setup/IconEntry.hpp"

#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace {

STORED_ENUM_MAP(StoredCloseOnExit, IconEntry::NoSetting,
	IconEntry::NoSetting,
	IconEntry::Yes,
	IconEntry::No,
);

} // anonymous namespace

void IconEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the icon entry structure
	}
	
	is >> encoded_string(name, version.codepage());
	is >> encoded_string(filename, version.codepage());
	is >> encoded_string(parameters, version.codepage());
	is >> encoded_string(workingDir, version.codepage());
	is >> encoded_string(iconFilename, version.codepage());
	is >> encoded_string(comment, version.codepage());
	
	loadConditionData(is, version);
	
	if(version >= INNO_VERSION(5, 3, 5)) {
		is >> encoded_string(appUserModelId, version.codepage());
	} else {
		appUserModelId.clear();
	}
	
	loadVersionData(is, version);
	
	iconIndex = load_number<int32_t>(is, version.bits);
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		showCmd = load_number<int32_t>(is);
		closeOnExit = stored_enum<StoredCloseOnExit>(is).get();
	} else {
		showCmd = 1, closeOnExit = NoSetting;
	}
	
	if(version >= INNO_VERSION(2, 0, 7)) {
		hotkey = load_number<uint16_t>(is);
	} else {
		hotkey = 0;
	}
	
	stored_flag_reader<Options> flags(is);
	
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

ENUM_NAMES(IconEntry::Options, "Icon Option",
	"never uninstall",
	"create only if file exists",
	"use app paths",
	"folder shortcut",
	"exclude from show in new install",
	"run minimized",
)

ENUM_NAMES(IconEntry::CloseOnExit, "Close on Exit",
	"no setting",
	"yes",
	"no",
)

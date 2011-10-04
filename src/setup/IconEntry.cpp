
#include "setup/IconEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

namespace {

STORED_ENUM_MAP(StoredCloseOnExit, IconEntry::NoSetting,
	IconEntry::NoSetting,
	IconEntry::Yes,
	IconEntry::No,
);

} // anonymous namespace

void IconEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<u32>(is); // uncompressed size of the icon entry structure
	}
	
	is >> EncodedString(name, version.codepage());
	is >> EncodedString(filename, version.codepage());
	is >> EncodedString(parameters, version.codepage());
	is >> EncodedString(workingDir, version.codepage());
	is >> EncodedString(iconFilename, version.codepage());
	is >> EncodedString(comment, version.codepage());
	
	loadConditionData(is, version);
	
	if(version >= INNO_VERSION(5, 3, 5)) {
		is >> EncodedString(appUserModelId, version.codepage());
	} else {
		appUserModelId.clear();
	}
	
	loadVersionData(is, version);
	
	iconIndex = loadNumber<s32>(is, version.bits);
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		showCmd = loadNumber<s32>(is);
		closeOnExit = StoredEnum<StoredCloseOnExit>(is).get();
	} else {
		showCmd = 1, closeOnExit = NoSetting;
	}
	
	if(version >= INNO_VERSION(2, 0, 7)) {
		hotkey = loadNumber<u16>(is);
	} else {
		hotkey = 0;
	}
	
	StoredFlagReader<Options> flags(is);
	
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
	
	options = flags.get();
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

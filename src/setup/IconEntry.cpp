
#include "setup/IconEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

STORED_ENUM_MAP(StoredCloseOnExit, IconEntry::NoSetting,
	IconEntry::NoSetting,
	IconEntry::Yes,
	IconEntry::No,
);

void IconEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version <= INNO_VERSION(1, 2, 16)) {
		::load<u32>(is); // uncompressed size of the icon entry structure
	}
	
	is >> EncodedString(name, version.codepage());
	is >> EncodedString(filename, version.codepage());
	is >> EncodedString(parameters, version.codepage());
	is >> EncodedString(workingDir, version.codepage());
	is >> EncodedString(iconFilename, version.codepage());
	is >> EncodedString(comment, version.codepage());
	condition.load(is, version);
	tasks.load(is, version);
	if(version >= INNO_VERSION(5, 3, 5)) {
		is >> EncodedString(appUserModelId, version.codepage());
	} else {
		appUserModelId.clear();
	}
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	iconIndex = loadNumber<s32>(is, version.bits);
	
	if(version > INNO_VERSION(1, 2, 16)) {
		showCmd = loadNumber<s32>(is);
		closeOnExit = StoredEnum<StoredCloseOnExit>(is).get();
	} else {
		showCmd = 0, closeOnExit = NoSetting;
	}
	
	if(version > INNO_VERSION(1, 3, 26)) {
		hotkey = loadNumber<u16>(is);
	}
	
	StoredFlagReader<IconFlags> flags(is);
	
	flags.add(ioUninsNeverUninstall);
	if(version > INNO_VERSION(1, 2, 16)) {
		flags.add(ioRunMinimized);
	}
	flags.add(ioCreateOnlyIfFileExists);
	if(version.bits != 16) {
		flags.add(ioUseAppPaths);
	}
	if(version >= INNO_VERSION(5, 0, 3)) {
		flags.add(ioFolderShortcut);
	}
	if(version >= INNO_VERSION(5, 4, 2)) {
		flags.add(ioExcludeFromShowInNewInstall);
	}
	
	options = flags.get();
}

ENUM_NAMES(IconFlags::Enum, "Icon Option",
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

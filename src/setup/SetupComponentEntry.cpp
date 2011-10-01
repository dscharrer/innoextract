
#include "setup/SetupComponentEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

STORED_FLAGS_MAP(StoredSetupComponentOptions0,
		coFixed,
		coRestart,
		coDisableNoUninstallWarning,
);

// starting with version 3.0.8
STORED_FLAGS_MAP(StoredSetupComponentOptions1,
		coFixed,
		coRestart,
		coDisableNoUninstallWarning,
		coExclusive,
);

// starting with version 4.2.3
STORED_FLAGS_MAP(StoredSetupComponentOptions2,
		coFixed,
		coRestart,
		coDisableNoUninstallWarning,
		coExclusive,
		coDontInheritCheck,
);

void SetupComponentEntry::load(std::istream & is, const InnoVersion & version) {
	
	is >> EncodedString(name, version.codepage());
	is >> EncodedString(description, version.codepage());
	is >> EncodedString(types, version.codepage());
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> EncodedString(languages, version.codepage());
	} else {
		languages.clear();
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		is >> EncodedString(check, version.codepage());
	} else {
		check.clear();
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		extraDiskSpaceRequired = loadNumber<u64>(is);
	} else {
		extraDiskSpaceRequired = loadNumber<u32>(is);
	}
	
	if(version >= INNO_VERSION(3, 0, 8)) {
		level = loadNumber<s32>(is);
		used = loadNumber<u8>(is);
	} else {
		level = 0, used = true;
	}
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	if(version >= INNO_VERSION(4, 2, 3)) {
		options = StoredFlags<StoredSetupComponentOptions2>(is).get();
	} else if(version >= INNO_VERSION(3, 0, 8)) {
		options = StoredFlags<StoredSetupComponentOptions1>(is).get();
	} else {
		options = StoredFlags<StoredSetupComponentOptions0>(is).get();
	}
	
	size = (version >= INNO_VERSION(4, 0, 0)) ? loadNumber<u64>(is) : loadNumber<u32>(is);
}

ENUM_NAMES(SetupComponentOptions::Enum, "Setup Component Option",
	"fixed",
	"restart",
	"disable no uninstall warning",
	"exclusive",
	"don't inherit check",
)

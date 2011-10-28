
#include "setup/SetupComponentEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

namespace {

STORED_FLAGS_MAP(StoredSetupComponentOptions0,
		SetupComponentEntry::Fixed,
		SetupComponentEntry::Restart,
		SetupComponentEntry::DisableNoUninstallWarning,
);

// starting with version 3.0.8
STORED_FLAGS_MAP(StoredSetupComponentOptions1,
		SetupComponentEntry::Fixed,
		SetupComponentEntry::Restart,
		SetupComponentEntry::DisableNoUninstallWarning,
		SetupComponentEntry::Exclusive,
);

// starting with version 4.2.3
STORED_FLAGS_MAP(StoredSetupComponentOptions2,
		SetupComponentEntry::Fixed,
		SetupComponentEntry::Restart,
		SetupComponentEntry::DisableNoUninstallWarning,
		SetupComponentEntry::Exclusive,
		SetupComponentEntry::DontInheritCheck,
);

} // anonymous namespace

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
		extraDiskSpaceRequired = loadNumber<uint64_t>(is);
	} else {
		extraDiskSpaceRequired = loadNumber<uint32_t>(is);
	}
	
	if(version >= INNO_VERSION(3, 0, 8)) {
		level = loadNumber<int32_t>(is);
		used = loadNumber<uint8_t>(is);
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
	
	size = (version >= INNO_VERSION(4, 0, 0)) ? loadNumber<uint64_t>(is) : loadNumber<uint32_t>(is);
}

ENUM_NAMES(SetupComponentEntry::Options, "Setup Component Option",
	"fixed",
	"restart",
	"disable no uninstall warning",
	"exclusive",
	"don't inherit check",
)

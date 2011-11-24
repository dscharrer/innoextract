
#include "setup/SetupComponentEntry.hpp"

#include "util/load.hpp"
#include "util/storedenum.hpp"

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
	
	is >> encoded_string(name, version.codepage());
	is >> encoded_string(description, version.codepage());
	is >> encoded_string(types, version.codepage());
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> encoded_string(languages, version.codepage());
	} else {
		languages.clear();
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		is >> encoded_string(check, version.codepage());
	} else {
		check.clear();
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		extraDiskSpaceRequired = load_number<uint64_t>(is);
	} else {
		extraDiskSpaceRequired = load_number<uint32_t>(is);
	}
	
	if(version >= INNO_VERSION(3, 0, 8)) {
		level = load_number<int32_t>(is);
		used = load_number<uint8_t>(is);
	} else {
		level = 0, used = true;
	}
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	if(version >= INNO_VERSION(4, 2, 3)) {
		options = stored_flags<StoredSetupComponentOptions2>(is).get();
	} else if(version >= INNO_VERSION(3, 0, 8)) {
		options = stored_flags<StoredSetupComponentOptions1>(is).get();
	} else {
		options = stored_flags<StoredSetupComponentOptions0>(is).get();
	}
	
	size = (version >= INNO_VERSION(4, 0, 0)) ? load_number<uint64_t>(is) : load_number<uint32_t>(is);
}

ENUM_NAMES(SetupComponentEntry::Options, "Setup Component Option",
	"fixed",
	"restart",
	"disable no uninstall warning",
	"exclusive",
	"don't inherit check",
)


#include "setup/SetupTaskEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

STORED_FLAGS_MAP(StoredSetupTaskOptions0,
	toExclusive,
	toUnchecked,
	toRestart,
	toCheckedOnce,
);

// starting with version 4.2.3
STORED_FLAGS_MAP(StoredSetupTaskOptions1,
	toExclusive,
	toUnchecked,
	toRestart,
	toCheckedOnce,
	toDontInheritCheck,
);

void SetupTaskEntry::load(std::istream & is, const InnoVersion & version) {
	
	is >> EncodedString(name, version.codepage());
	is >> EncodedString(description, version.codepage());
	is >> EncodedString(groupDescription, version.codepage());
	is >> EncodedString(components, version.codepage());
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> EncodedString(languages, version.codepage());
	} else {
		languages.clear();
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		is >> EncodedString(check, version.codepage());
		level = loadNumber<s32>(is);
		used = loadNumber<u8>(is);
	} else {
		check.clear(), level = 0, used = true;
	}
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	if(version >= INNO_VERSION(4, 2, 3)) {
		options = StoredFlags<StoredSetupTaskOptions1>(is).get();
	} else {
		options = StoredFlags<StoredSetupTaskOptions0>(is).get();
	}
}

ENUM_NAMES(SetupTaskOptions::Enum, "Setup Task Option",
	"exclusive",
	"unchecked",
	"restart",
	"checked once",
	"don't inherit check",
)

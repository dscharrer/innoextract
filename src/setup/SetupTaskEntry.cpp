
#include "setup/SetupTaskEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

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
	
	StoredFlagReader<Options> flags(is);
	
	flags.add(Exclusive);
	flags.add(Unchecked);
	if(version >= INNO_VERSION(2, 0, 5)) {
		flags.add(Restart);
	}
	if(version >= INNO_VERSION(2, 0, 6)) {
		flags.add(CheckedOnce);
	}
	if(version >= INNO_VERSION(4, 2, 3)) {
		flags.add(DontInheritCheck);
	}
	
	options = flags.get();
}

ENUM_NAMES(SetupTaskEntry::Options, "Setup Task Option",
	"exclusive",
	"unchecked",
	"restart",
	"checked once",
	"don't inherit check",
)

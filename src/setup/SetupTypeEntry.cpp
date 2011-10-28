
#include "setup/SetupTypeEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

namespace {

STORED_FLAGS_MAP(StoredSetupTypeOptions,
	SetupTypeEntry::CustomSetupType,
);

STORED_ENUM_MAP(StoredSetupType, SetupTypeEntry::User,
	SetupTypeEntry::User,
	SetupTypeEntry::DefaultFull,
	SetupTypeEntry::DefaultCompact,
	SetupTypeEntry::DefaultCustom,
);

} // anonymous namespace

void SetupTypeEntry::load(std::istream & is, const InnoVersion & version) {
	
	is >> EncodedString(name, version.codepage());
	is >> EncodedString(description, version.codepage());
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
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	options = StoredFlags<StoredSetupTypeOptions>(is).get();
	
	if(version >= INNO_VERSION(4, 0, 3)) {
		type = StoredEnum<StoredSetupType>(is).get();
	} else {
		type = User;
	}
	
	size = (version >= INNO_VERSION(4, 0, 0)) ? loadNumber<uint64_t>(is) : loadNumber<uint32_t>(is);
}

ENUM_NAMES(SetupTypeEntry::Options, "Setyp Type Option",
	"is custom",
)

ENUM_NAMES(SetupTypeEntry::Type, "Setyp Type",
	"user",
	"default full",
	"default compact",
	"default custom",
)
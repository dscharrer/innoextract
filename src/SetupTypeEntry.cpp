
#include "SetupTypeEntry.hpp"

#include "StoredEnum.hpp"
#include "LoadingUtils.hpp"

STORED_FLAGS_MAP(StoredSetupTypeOptions,
	CustomSetupType,
);

STORED_ENUM_MAP(StoredSetupType, SetupTypeEntry::User,
	SetupTypeEntry::User,
	SetupTypeEntry::DefaultFull,
	SetupTypeEntry::DefaultCompact,
	SetupTypeEntry::DefaultCustom,
);

void SetupTypeEntry::load(std::istream & is, const InnoVersion & version) {
	
	is >> EncodedString(name, version.codepage());
	is >> EncodedString(description, version.codepage());
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> EncodedString(langauges, version.codepage());
	} else {
		langauges.clear();
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
	
	size = (version >= INNO_VERSION(4, 0, 0)) ? loadNumber<u64>(is) : loadNumber<u32>(is);
}

ENUM_NAMES(SetupTypeOptions::Enum, "Setyp Type Option",
	"is custom",
)

ENUM_NAMES(SetupTypeEntry::Type, "Setyp Type",
	"user",
	"default full",
	"default compact",
	"default custom",
)
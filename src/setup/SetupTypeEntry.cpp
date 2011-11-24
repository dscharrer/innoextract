
#include "setup/SetupTypeEntry.hpp"

#include "util/load.hpp"
#include "util/storedenum.hpp"

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
	
	is >> encoded_string(name, version.codepage());
	is >> encoded_string(description, version.codepage());
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
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	options = stored_flags<StoredSetupTypeOptions>(is).get();
	
	if(version >= INNO_VERSION(4, 0, 3)) {
		type = stored_enum<StoredSetupType>(is).get();
	} else {
		type = User;
	}
	
	size = (version >= INNO_VERSION(4, 0, 0)) ? load_number<uint64_t>(is) : load_number<uint32_t>(is);
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

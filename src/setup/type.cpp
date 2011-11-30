
#include "setup/type.hpp"

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

FLAGS(type_flags,
	CustomSetupType
);

STORED_FLAGS_MAP(stored_type_flags,
	CustomSetupType,
);

STORED_ENUM_MAP(stored_setup_type, type_entry::User,
	type_entry::User,
	type_entry::DefaultFull,
	type_entry::DefaultCompact,
	type_entry::DefaultCustom,
);

} // anonymous namespace

} // namespace setup

NAMED_FLAGS(setup::type_flags)

namespace setup {

void type_entry::load(std::istream & is, const version & version) {
	
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
	
	winver.load(is, version);
	
	type_flags options = stored_flags<stored_type_flags>(is).get();
	custom_type = (options & CustomSetupType);
	
	if(version >= INNO_VERSION(4, 0, 3)) {
		type = stored_enum<stored_setup_type>(is).get();
	} else {
		type = User;
	}
	
	size = (version >= INNO_VERSION(4, 0, 0)) ? load_number<uint64_t>(is) : load_number<uint32_t>(is);
}

} // namespace setup

NAMES(setup::type_flags, "Setyp Type Option",
	"is custom",
)

NAMES(setup::type_entry::setup_type, "Setyp Type",
	"user",
	"default full",
	"default compact",
	"default custom",
)

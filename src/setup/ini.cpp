
#include "setup/ini.hpp"

#include <stdint.h>

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

STORED_FLAGS_MAP(stored_ini_flags,
	ini_entry::CreateKeyIfDoesntExist,
	ini_entry::UninsDeleteEntry,
	ini_entry::UninsDeleteEntireSection,
	ini_entry::UninsDeleteSectionIfEmpty,
	ini_entry::HasValue,
);

} // anonymous namespace

void ini_entry::load(std::istream & is, const version & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the ini entry structure
	}
	
	is >> encoded_string(inifile, version.codepage());
	if(inifile.empty()) {
		inifile = "{windows}/WIN.INI";
	}
	is >> encoded_string(section, version.codepage());
	is >> encoded_string(key, version.codepage());
	is >> encoded_string(value, version.codepage());
	
	load_condition_data(is, version);
	
	load_version_data(is, version);
	
	options = stored_flags<stored_ini_flags>(is).get();
}

} // namespace setup

NAMES(setup::ini_entry::flags, "Ini Option",
	"create key if doesn't exist",
	"uninstall delete entry",
	"uninstall delete section",
	"uninstall delete section if empty",
	"has value",
)

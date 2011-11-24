
#include "setup/IniEntry.hpp"

#include <stdint.h>

#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace {

STORED_FLAGS_MAP(StoredIniOptions,
	IniEntry::CreateKeyIfDoesntExist,
	IniEntry::UninsDeleteEntry,
	IniEntry::UninsDeleteEntireSection,
	IniEntry::UninsDeleteSectionIfEmpty,
	IniEntry::HasValue,
);

} // anonymous namespace

void IniEntry::load(std::istream & is, const InnoVersion & version) {
	
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
	
	loadConditionData(is, version);
	
	loadVersionData(is, version);
	
	options = stored_flags<StoredIniOptions>(is).get();
}

ENUM_NAMES(IniEntry::Options, "Ini Option",
	"create key if doesn't exist",
	"uninstall delete entry",
	"uninstall delete section",
	"uninstall delete section if empty",
	"has value",
)

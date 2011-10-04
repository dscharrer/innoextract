
#include "setup/IniEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

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
		::load<u32>(is); // uncompressed size of the ini entry structure
	}
	
	is >> EncodedString(inifile, version.codepage());
	if(inifile.empty()) {
		inifile = "{windows}/WIN.INI";
	}
	is >> EncodedString(section, version.codepage());
	is >> EncodedString(key, version.codepage());
	is >> EncodedString(value, version.codepage());
	
	loadConditionData(is, version);
	
	loadVersionData(is, version);
	
	options = StoredFlags<StoredIniOptions>(is).get();
}

ENUM_NAMES(IniEntry::Options, "Ini Option",
	"create key if doesn't exist",
	"uninstall delete entry",
	"uninstall delete section",
	"uninstall delete section if empty",
	"has value",
)

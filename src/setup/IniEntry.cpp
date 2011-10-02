
#include "setup/IniEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

namespace {

STORED_FLAGS_MAP(StoredIniOptions,
	ioCreateKeyIfDoesntExist,
	ioUninsDeleteEntry,
	ioUninsDeleteEntireSection,
	ioUninsDeleteSectionIfEmpty,
	ioHasValue,
);

} // anonymous namespace

void IniEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version <= INNO_VERSION(1, 2, 16)) {
		::load<u32>(is); // uncompressed size of the ini entry structure
	}
	
	is >> EncodedString(inifile, version.codepage());
	if(inifile.empty()) {
		inifile = "{windows}/WIN.INI";
	}
	is >> EncodedString(section, version.codepage());
	is >> EncodedString(key, version.codepage());
	is >> EncodedString(value, version.codepage());
	condition.load(is, version);
	tasks.load(is, version);
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	options = StoredFlags<StoredIniOptions>(is).get();
}

ENUM_NAMES(IniOptions::Enum, "Ini Option",
	"create key if doesn't exist",
	"uninstall delete entry",
	"uninstall delete section",
	"uninstall delete section if empty",
	"has value",
)

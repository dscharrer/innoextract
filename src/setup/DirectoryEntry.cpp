
#include "setup/DirectoryEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

STORED_FLAGS_MAP(StoredInnoDirectoryOptions0,
	doUninsNeverUninstall,
	doDeleteAfterInstall,
	doUninsAlwaysUninstall,
);

// starting with version 5.2.0
STORED_FLAGS_MAP(StoredInnoDirectoryOptions1,
	doUninsNeverUninstall,
	doDeleteAfterInstall,
	doUninsAlwaysUninstall,
	doSetNTFSCompression,
	doUnsetNTFSCompression,
);

void DirectoryEntry::load(std::istream & is, const InnoVersion & version) {
	
	is >> EncodedString(name, version.codepage());
	if(version > INNO_VERSION(1, 3, 26)) {
		is >> EncodedString(components, version.codepage());
		is >> EncodedString(tasks, version.codepage());
	} else {
		components.clear(), tasks.clear();
	}
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
	if(version >= INNO_VERSION(4, 0, 11) && version < INNO_VERSION(4, 1, 0)) {
		is >> EncodedString(permissions, version.codepage());
	} else {
		permissions.clear();
	}
	if(version >= INNO_VERSION(4, 1, 0)) {
		is >> EncodedString(afterInstall, version.codepage());
		is >> EncodedString(beforeInstall, version.codepage());
	} else {
		afterInstall.clear(), beforeInstall.clear();
	}
	
	if(version >= INNO_VERSION(2, 0, 11)) {
		attributes = loadNumber<u32>(is);
	} else {
		attributes = 0;
	}
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission = loadNumber<s16>(is);
	} else {
		permission = -1;
	}
	
	if(version >= INNO_VERSION(5, 2, 0)) {
		options = StoredFlags<StoredInnoDirectoryOptions1>(is).get();
	} else {
		options = StoredFlags<StoredInnoDirectoryOptions0>(is).get();
	}
	
}

ENUM_NAMES(InnoDirectoryOptions::Enum, "Directory Option",
	"never uninstall",
	"delete after install",
	"always uninstall",
	"set NTFS compression",
	"unset NTFS compression",
)

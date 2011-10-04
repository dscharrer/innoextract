
#include "setup/DirectoryEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

namespace {

STORED_FLAGS_MAP(StoredInnoDirectoryOptions0,
	DirectoryEntry::NeverUninstall,
	DirectoryEntry::DeleteAfterInstall,
	DirectoryEntry::AlwaysUninstall,
);

// starting with version 5.2.0
STORED_FLAGS_MAP(StoredInnoDirectoryOptions1,
	DirectoryEntry::NeverUninstall,
	DirectoryEntry::DeleteAfterInstall,
	DirectoryEntry::AlwaysUninstall,
	DirectoryEntry::SetNtfsCompression,
	DirectoryEntry::UnsetNtfsCompression,
);

} // anonymous namespace

void DirectoryEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<u32>(is); // uncompressed size of the directory entry structure
	}
	
	is >> EncodedString(name, version.codepage());
	
	loadConditionData(is, version);
	
	if(version >= INNO_VERSION(4, 0, 11) && version < INNO_VERSION(4, 1, 0)) {
		is >> EncodedString(permissions, version.codepage());
	} else {
		permissions.clear();
	}
	
	if(version >= INNO_VERSION(2, 0, 11)) {
		attributes = loadNumber<u32>(is);
	} else {
		attributes = 0;
	}
	
	loadVersionData(is, version);
	
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

ENUM_NAMES(DirectoryEntry::Options, "Directory Option",
	"never uninstall",
	"delete after install",
	"always uninstall",
	"set NTFS compression",
	"unset NTFS compression",
)

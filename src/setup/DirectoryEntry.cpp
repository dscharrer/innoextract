
#include "setup/DirectoryEntry.hpp"

#include "util/load.hpp"
#include "util/storedenum.hpp"

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
		::load<uint32_t>(is); // uncompressed size of the directory entry structure
	}
	
	is >> encoded_string(name, version.codepage());
	
	loadConditionData(is, version);
	
	if(version >= INNO_VERSION(4, 0, 11) && version < INNO_VERSION(4, 1, 0)) {
		is >> encoded_string(permissions, version.codepage());
	} else {
		permissions.clear();
	}
	
	if(version >= INNO_VERSION(2, 0, 11)) {
		attributes = load_number<uint32_t>(is);
	} else {
		attributes = 0;
	}
	
	loadVersionData(is, version);
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission = load_number<int16_t>(is);
	} else {
		permission = -1;
	}
	
	if(version >= INNO_VERSION(5, 2, 0)) {
		options = stored_flags<StoredInnoDirectoryOptions1>(is).get();
	} else {
		options = stored_flags<StoredInnoDirectoryOptions0>(is).get();
	}
	
}

ENUM_NAMES(DirectoryEntry::Options, "Directory Option",
	"never uninstall",
	"delete after install",
	"always uninstall",
	"set NTFS compression",
	"unset NTFS compression",
)


#include "setup/directory.hpp"

#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

STORED_FLAGS_MAP(stored_inno_directory_options_0,
	directory_entry::NeverUninstall,
	directory_entry::DeleteAfterInstall,
	directory_entry::AlwaysUninstall,
);

// starting with version 5.2.0
STORED_FLAGS_MAP(stored_inno_directory_options_1,
	directory_entry::NeverUninstall,
	directory_entry::DeleteAfterInstall,
	directory_entry::AlwaysUninstall,
	directory_entry::SetNtfsCompression,
	directory_entry::UnsetNtfsCompression,
);

} // anonymous namespace

void directory_entry::load(std::istream & is, const inno_version & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the directory entry structure
	}
	
	is >> encoded_string(name, version.codepage());
	
	load_condition_data(is, version);
	
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
	
	load_version_data(is, version);
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission = load_number<int16_t>(is);
	} else {
		permission = -1;
	}
	
	if(version >= INNO_VERSION(5, 2, 0)) {
		options = stored_flags<stored_inno_directory_options_1>(is).get();
	} else {
		options = stored_flags<stored_inno_directory_options_0>(is).get();
	}
	
}

} // namespace setup

ENUM_NAMES(setup::directory_entry::flags, "Directory Option",
	"never uninstall",
	"delete after install",
	"always uninstall",
	"set NTFS compression",
	"unset NTFS compression",
)

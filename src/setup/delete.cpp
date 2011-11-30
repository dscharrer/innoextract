
#include "setup/delete.hpp"

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

STORED_ENUM_MAP(delete_target_type_map, delete_entry::Files,
	delete_entry::Files,
	delete_entry::FilesAndSubdirs,
	delete_entry::DirIfEmpty,
);

} // anonymous namespace

void delete_entry::load(std::istream & is, const version & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the directory entry structure
	}
	
	is >> encoded_string(name, version.codepage());
	
	load_condition_data(is, version);
	
	load_version_data(is, version);
	
	type = stored_enum<delete_target_type_map>(is).get();
}

} // namespace setup

NAMES(setup::delete_entry::target_type, "Delete Type",
	"files",
	"files and subdirs",
	"dir if empty",
)

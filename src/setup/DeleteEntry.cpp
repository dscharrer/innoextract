
#include "setup/DeleteEntry.hpp"

#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace {

STORED_ENUM_MAP(DeleteTypeMap, DeleteEntry::Files,
	DeleteEntry::Files,
	DeleteEntry::FilesAndSubdirs,
	DeleteEntry::DirIfEmpty,
);

} // anonymous namespace

void DeleteEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the directory entry structure
	}
	
	is >> encoded_string(name, version.codepage());
	
	loadConditionData(is, version);
	
	loadVersionData(is, version);
	
	type = stored_enum<DeleteTypeMap>(is).get();
}

ENUM_NAMES(DeleteEntry::Type, "Delete Type",
	"files",
	"files and subdirs",
	"dir if empty",
)

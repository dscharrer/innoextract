
#include "setup/DeleteEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

namespace {

STORED_ENUM_MAP(DeleteTypeMap, DeleteEntry::Files,
	DeleteEntry::Files,
	DeleteEntry::FilesAndSubdirs,
	DeleteEntry::DirIfEmpty,
);

} // anonymous namespace

void DeleteEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<u32>(is); // uncompressed size of the directory entry structure
	}
	
	is >> EncodedString(name, version.codepage());
	
	loadConditionData(is, version);
	
	loadVersionData(is, version);
	
	type = StoredEnum<DeleteTypeMap>(is).get();
}

ENUM_NAMES(DeleteEntry::Type, "Delete Type",
	"files",
	"files and subdirs",
	"dir if empty",
)

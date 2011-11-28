
#ifndef INNOEXTRACT_SETUP_DELETE_HPP
#define INNOEXTRACT_SETUP_DELETE_HPP

#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/version.hpp"
#include "util/enum.hpp"

namespace setup {

struct delete_entry : public SetupItem {
	
	enum target_type {
		Files,
		FilesAndSubdirs,
		DirIfEmpty,
	};
	
	std::string name;
	
	target_type type;
	
	void load(std::istream & is, const inno_version & version);
	
};

} // namespace setup

NAMED_ENUM(setup::delete_entry::target_type)

#endif // INNOEXTRACT_SETUP_DELETE_HPP

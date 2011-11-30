
#ifndef INNOEXTRACT_SETUP_INI_HPP
#define INNOEXTRACT_SETUP_INI_HPP

#include <string>
#include <iosfwd>

#include "setup/item.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

struct ini_entry : public item {
	
	FLAGS(flags,
		CreateKeyIfDoesntExist,
		UninsDeleteEntry,
		UninsDeleteEntireSection,
		UninsDeleteSectionIfEmpty,
		HasValue
	);
	
	std::string inifile;
	std::string section;
	std::string key;
	std::string value;
	
	flags options;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_FLAGS(setup::ini_entry::flags)

#endif // INNOEXTRACT_SETUP_INI_HPP

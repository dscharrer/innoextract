
#ifndef INNOEXTRACT_SETUP_INIENTRY_HPP
#define INNOEXTRACT_SETUP_INIENTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/version.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

struct IniEntry : public SetupItem {
	
	FLAGS(Options,
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
	
	Options options;
	
	void load(std::istream & is, const inno_version & version);
	
};

FLAGS_OVERLOADS(IniEntry::Options)
NAMED_ENUM(IniEntry::Options)

#endif // INNOEXTRACT_SETUP_INIENTRY_HPP

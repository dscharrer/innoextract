
#ifndef INNOEXTRACT_SETUP_INIENTRY_HPP
#define INNOEXTRACT_SETUP_INIENTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/Version.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"

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
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(IniEntry::Options)
NAMED_ENUM(IniEntry::Options)

#endif // INNOEXTRACT_SETUP_INIENTRY_HPP

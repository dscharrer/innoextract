
#ifndef INNOEXTRACT_SETUP_SETUPCOMPONENTENTRY_HPP
#define INNOEXTRACT_SETUP_SETUPCOMPONENTENTRY_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

struct SetupComponentEntry {
	
	// introduced in 2.0.0
	
	FLAGS(Options,
		Fixed,
		Restart,
		DisableNoUninstallWarning,
		Exclusive,
		DontInheritCheck
	);
	
	std::string name;
	std::string description;
	std::string types;
	std::string languages;
	std::string check;
	
	uint64_t extraDiskSpaceRequired;
	
	int level;
	bool used;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	Options options;
	
	uint64_t size;
	
	void load(std::istream & is, const inno_version & version);
	
};

FLAGS_OVERLOADS(SetupComponentEntry::Options)
NAMED_ENUM(SetupComponentEntry::Options)

#endif // INNOEXTRACT_SETUP_SETUPCOMPONENTENTRY_HPP

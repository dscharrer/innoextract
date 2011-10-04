
#ifndef INNOEXTRACT_SETUP_SETUPCOMPONENTENTRY_HPP
#define INNOEXTRACT_SETUP_SETUPCOMPONENTENTRY_HPP

#include <iostream>

#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"

#include "util/Enum.hpp"
#include "util/Flags.hpp"

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
	
	u64 extraDiskSpaceRequired;
	
	int level;
	bool used;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	Options options;
	
	u64 size;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(SetupComponentEntry::Options)
NAMED_ENUM(SetupComponentEntry::Options)

#endif // INNOEXTRACT_SETUP_SETUPCOMPONENTENTRY_HPP

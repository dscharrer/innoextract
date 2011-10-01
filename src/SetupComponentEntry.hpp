
#ifndef INNOEXTRACT_SETUPCOMPONENTENTRY_HPP
#define INNOEXTRACT_SETUPCOMPONENTENTRY_HPP

#include <iostream>
#include "Version.hpp"
#include "WindowsVersion.hpp"
#include "Flags.hpp"
#include "Enum.hpp"

FLAGS(SetupComponentOptions,
	coFixed,
	coRestart,
	coDisableNoUninstallWarning,
	coExclusive,
	coDontInheritCheck,
)

NAMED_ENUM(SetupComponentOptions::Enum)

struct SetupComponentEntry {
	
	// introduced after 1.3.26
	
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
	
	SetupComponentOptions options;
	
	u64 size;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_SETUPCOMPONENTENTRY_HPP


#ifndef INNOEXTRACT_SETUP_SETUPTASKENTRY_HPP
#define INNOEXTRACT_SETUP_SETUPTASKENTRY_HPP

#include <iostream>

#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"

FLAGS(SetupTaskOptions,
	toExclusive,
	toUnchecked,
	toRestart,
	toCheckedOnce,
	toDontInheritCheck,
)

NAMED_ENUM(SetupTaskOptions::Enum)

struct SetupTaskEntry {
	
	// introduced after 1.3.26
	
	std::string name;
	std::string description;
	std::string groupDescription;
	std::string components;
	std::string languages;
	std::string check;
	
	int level;
	bool used;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	SetupTaskOptions options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_SETUP_SETUPTASKENTRY_HPP

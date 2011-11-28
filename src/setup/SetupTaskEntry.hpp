
#ifndef INNOEXTRACT_SETUP_SETUPTASKENTRY_HPP
#define INNOEXTRACT_SETUP_SETUPTASKENTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

struct SetupTaskEntry {
	
	// introduced in 2.0.0
	
	FLAGS(Options,
		Exclusive,
		Unchecked,
		Restart,
		CheckedOnce,
		DontInheritCheck
	);
	
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
	
	Options options;
	
	void load(std::istream & is, const inno_version & version);
	
};

FLAGS_OVERLOADS(SetupTaskEntry::Options)
NAMED_ENUM(SetupTaskEntry::Options)

#endif // INNOEXTRACT_SETUP_SETUPTASKENTRY_HPP

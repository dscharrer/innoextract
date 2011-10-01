
#ifndef INNOEXTRACT_SETUP_SETUPTYPEENTRY_HPP
#define INNOEXTRACT_SETUP_SETUPTYPEENTRY_HPP

#include <iostream>

#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"

FLAGS(SetupTypeOptions,
	CustomSetupType,
)

NAMED_ENUM(SetupTypeOptions::Enum)

struct SetupTypeEntry {
	
	// introduced after 1.3.26
	
	enum Type {
		User,
		DefaultFull,
		DefaultCompact,
		DefaultCustom
	};
	
	std::string name;
	std::string description;
	std::string languages;
	std::string check;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	SetupTypeOptions options;
	
	Type type;
	
	u64 size;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

NAMED_ENUM(SetupTypeEntry::Type)

#endif // INNOEXTRACT_SETUP_SETUPTYPEENTRY_HPP

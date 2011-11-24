
#ifndef INNOEXTRACT_SETUP_SETUPTYPEENTRY_HPP
#define INNOEXTRACT_SETUP_SETUPTYPEENTRY_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

struct SetupTypeEntry {
	
	// introduced in 2.0.0
	
	FLAGS(Options,
		CustomSetupType
	);
	
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
	
	Options options;
	
	Type type;
	
	uint64_t size;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(SetupTypeEntry::Options)
NAMED_ENUM(SetupTypeEntry::Options)

NAMED_ENUM(SetupTypeEntry::Type)

#endif // INNOEXTRACT_SETUP_SETUPTYPEENTRY_HPP

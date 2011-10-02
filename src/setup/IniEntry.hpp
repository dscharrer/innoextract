
#ifndef INNOEXTRACT_SETUP_INIENTRY_HPP
#define INNOEXTRACT_SETUP_INIENTRY_HPP

#include <iostream>

#include "setup/SetupCondition.hpp"
#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"
#include "util/Types.hpp"

FLAGS(IniOptions,
	ioCreateKeyIfDoesntExist,
	ioUninsDeleteEntry,
	ioUninsDeleteEntireSection,
	ioUninsDeleteSectionIfEmpty,
	ioHasValue,
)

NAMED_ENUM(IniOptions::Enum)

struct IniEntry {
	
	std::string inifile;
	std::string section;
	std::string key;
	std::string value;
	SetupCondition condition;
	SetupTasks tasks;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	IniOptions options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_SETUP_INIENTRY_HPP


#ifndef INNOEXTRACT_SETUP_DIRECTORYENTRY_HPP
#define INNOEXTRACT_SETUP_DIRECTORYENTRY_HPP

#include <iostream>

#include "setup/SetupCondition.hpp"
#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"
#include "util/Types.hpp"

FLAGS(InnoDirectoryOptions,
	doUninsNeverUninstall,
	doDeleteAfterInstall,
	doUninsAlwaysUninstall,
	doSetNTFSCompression,
	doUnsetNTFSCompression,
)

NAMED_ENUM(InnoDirectoryOptions::Enum)

struct DirectoryEntry {
	
	std::string name;
	SetupCondition condition;
	SetupTasks tasks;
	std::string permissions;
	
	u32 attributes;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	int permission; //!< index into the permission entry list
	
	InnoDirectoryOptions options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_SETUP_DIRECTORYENTRY_HPP

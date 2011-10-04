
#ifndef INNOEXTRACT_SETUP_DIRECTORYENTRY_HPP
#define INNOEXTRACT_SETUP_DIRECTORYENTRY_HPP

#include <iostream>

#include "setup/SetupItem.hpp"
#include "setup/Version.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"
#include "util/Types.hpp"

struct DirectoryEntry : public SetupItem {
	
	FLAGS(Options,
		NeverUninstall,
		DeleteAfterInstall,
		AlwaysUninstall,
		SetNtfsCompression,
		UnsetNtfsCompression
	);
	
	std::string name;
	std::string permissions;
	
	u32 attributes;
	
	int permission; //!< index into the permission entry list
	
	Options options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(DirectoryEntry::Options)
NAMED_ENUM(DirectoryEntry::Options)

#endif // INNOEXTRACT_SETUP_DIRECTORYENTRY_HPP


#ifndef INNOEXTRACT_SETUP_DIRECTORY_HPP
#define INNOEXTRACT_SETUP_DIRECTORY_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/version.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct directory_entry : public SetupItem {
	
	FLAGS(flags,
		NeverUninstall,
		DeleteAfterInstall,
		AlwaysUninstall,
		SetNtfsCompression,
		UnsetNtfsCompression
	);
	
	std::string name;
	std::string permissions;
	
	uint32_t attributes;
	
	int permission; //!< index into the permission entry list
	
	flags options;
	
	void load(std::istream & is, const inno_version & version);
	
};

} // namespace setup

FLAGS_OVERLOADS(setup::directory_entry::flags)
NAMED_ENUM(setup::directory_entry::flags)

#endif // INNOEXTRACT_SETUP_DIRECTORY_HPP

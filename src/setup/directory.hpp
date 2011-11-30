
#ifndef INNOEXTRACT_SETUP_DIRECTORY_HPP
#define INNOEXTRACT_SETUP_DIRECTORY_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/item.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

struct directory_entry : public item {
	
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
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_FLAGS(setup::directory_entry::flags)

#endif // INNOEXTRACT_SETUP_DIRECTORY_HPP

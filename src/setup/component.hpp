
#ifndef INNOEXTRACT_SETUP_COMPONENT_HPP
#define INNOEXTRACT_SETUP_COMPONENT_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/windows.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

struct component_entry {
	
	// introduced in 2.0.0
	
	FLAGS(flags,
		Fixed,
		Restart,
		DisableNoUninstallWarning,
		Exclusive,
		DontInheritCheck
	);
	
	std::string name;
	std::string description;
	std::string types;
	std::string languages;
	std::string check;
	
	uint64_t extra_disk_pace_required;
	
	int level;
	bool used;
	
	windows_version_range winver;
	
	flags options;
	
	uint64_t size;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_FLAGS(setup::component_entry::flags)

#endif // INNOEXTRACT_SETUP_COMPONENT_HPP


#ifndef INNOEXTRACT_SETUP_TASK_HPP
#define INNOEXTRACT_SETUP_TASK_HPP

#include <string>
#include <iosfwd>

#include "setup/windows.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

struct task_entry {
	
	// introduced in 2.0.0
	
	FLAGS(flags,
		Exclusive,
		Unchecked,
		Restart,
		CheckedOnce,
		DontInheritCheck
	);
	
	std::string name;
	std::string description;
	std::string group_description;
	std::string components;
	std::string languages;
	std::string check;
	
	int level;
	bool used;
	
	windows_version_range winver;
	
	flags options;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_FLAGS(setup::task_entry::flags)

#endif // INNOEXTRACT_SETUP_TASK_HPP

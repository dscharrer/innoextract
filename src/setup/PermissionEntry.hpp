
#ifndef INNOEXTRACT_SETUP_PERMISSIONENTRY_HPP
#define INNOEXTRACT_SETUP_PERMISSIONENTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/version.hpp"

struct PermissionEntry {
	
	// introduced in 4.1.0
	
	std::string permissions;
	
	void load(std::istream & is, const inno_version & version);
	
};

#endif // INNOEXTRACT_SETUP_PERMISSIONENTRY_HPP

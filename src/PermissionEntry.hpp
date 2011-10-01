
#ifndef INNOEXTRACT_PERMISSIONENTRY_HPP
#define INNOEXTRACT_PERMISSIONENTRY_HPP

#include <iostream>
#include "Version.hpp"

struct PermissionEntry {
	
	// introduced in version 4.1.0
	
	std::string permissions;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_PERMISSIONENTRY_HPP

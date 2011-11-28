
#include "setup/PermissionEntry.hpp"

#include "util/load.hpp"

void PermissionEntry::load(std::istream & is, const inno_version & version) {
	
	(void)version;
	
	is >> binary_string(permissions); // an array of TGrantPermissionEntry's
	
}

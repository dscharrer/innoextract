
#include "setup/PermissionEntry.hpp"

#include "util/LoadingUtils.hpp"

void PermissionEntry::load(std::istream & is, const InnoVersion & version) {
	
	(void)version;
	
	is >> BinaryString(permissions); // an array of TGrantPermissionEntry's
	
}

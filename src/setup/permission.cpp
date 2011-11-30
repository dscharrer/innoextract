
#include "setup/permission.hpp"

#include "util/load.hpp"

namespace setup {

void permission_entry::load(std::istream & is, const version & version) {
	
	(void)version;
	
	is >> binary_string(permissions); // an array of TGrantPermissionEntry's
	
}

} // namespace setup

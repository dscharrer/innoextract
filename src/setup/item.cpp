
#include "setup/item.hpp"

#include "setup/version.hpp"
#include "util/load.hpp"

namespace setup {

void item::load_condition_data(std::istream & is, const version & version) {
	
	if(version >= INNO_VERSION(2, 0, 0)) {
		is >> encoded_string(components, version.codepage());
		is >> encoded_string(tasks, version.codepage());
	} else {
		components.clear(), tasks.clear();
	}
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> encoded_string(languages, version.codepage());
	} else {
		languages.clear();
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		is >> encoded_string(check, version.codepage());
	} else {
		check.clear();
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		is >> encoded_string(after_install, version.codepage());
		is >> encoded_string(before_install, version.codepage());
	} else {
		after_install.clear(), before_install.clear();
	}
	
}

} // namespace setup

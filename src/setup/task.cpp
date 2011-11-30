
#include "setup/task.hpp"

#include <stdint.h>

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

void task_entry::load(std::istream & is, const version & version) {
	
	is >> encoded_string(name, version.codepage());
	is >> encoded_string(description, version.codepage());
	is >> encoded_string(group_description, version.codepage());
	is >> encoded_string(components, version.codepage());
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> encoded_string(languages, version.codepage());
	} else {
		languages.clear();
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		is >> encoded_string(check, version.codepage());
		level = load_number<int32_t>(is);
		used = load_number<uint8_t>(is);
	} else {
		check.clear(), level = 0, used = true;
	}
	
	winver.load(is, version);
	
	stored_flag_reader<flags> flags(is);
	
	flags.add(Exclusive);
	flags.add(Unchecked);
	if(version >= INNO_VERSION(2, 0, 5)) {
		flags.add(Restart);
	}
	if(version >= INNO_VERSION(2, 0, 6)) {
		flags.add(CheckedOnce);
	}
	if(version >= INNO_VERSION(4, 2, 3)) {
		flags.add(DontInheritCheck);
	}
	
	options = flags;
}

} // namespace setup

NAMES(setup::task_entry::flags, "Setup Task Option",
	"exclusive",
	"unchecked",
	"restart",
	"checked once",
	"don't inherit check",
)

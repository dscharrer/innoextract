
#include "setup/message.hpp"

#include <stdint.h>

#include "setup/version.hpp"
#include "util/load.hpp"

namespace setup {

void message_entry::load(std::istream & is, const version & version) {
	
	is >> encoded_string(name, version.codepage());
	is >> binary_string(value); // encoding depends on the codepage in the LanguageEntry
	
	language = load_number<int32_t>(is);
	
}

} // namespace setup

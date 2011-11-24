
#include "setup/MessageEntry.hpp"

#include <stdint.h>

#include "util/load.hpp"

void MessageEntry::load(std::istream & is, const InnoVersion & version) {
	
	is >> encoded_string(name, version.codepage());
	is >> binary_string(value); // encoding depends on the codepage in the LanguageEntry
	
	language = load_number<int32_t>(is);
	
}

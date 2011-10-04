
#include "setup/MessageEntry.hpp"

#include "util/LoadingUtils.hpp"

void MessageEntry::load(std::istream & is, const InnoVersion & version) {
	
	is >> EncodedString(name, version.codepage());
	is >> BinaryString(value); // encoding depends on the codepage in the LanguageEntry
	
	language = loadNumber<s32>(is);
	
}

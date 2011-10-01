
#include "CustomMessageEntry.hpp"

#include "LoadingUtils.hpp"

void CustomMessageEntry::load(std::istream & is, const InnoVersion & version) {
	
	is >> EncodedString(name, version.codepage());
	is >> BinaryString(value);
	
	language = loadNumber<s32>(is);
	
}

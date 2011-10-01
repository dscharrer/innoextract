
#include "LanguageEntry.hpp"

#include <sstream>
#include <iconv.h>
#include "LoadingUtils.hpp"
#include "Output.hpp"

void convert(iconv_t converter, const std::string & from, std::string & to);

void LanguageEntry::load(std::istream & is, const InnoVersion & version) {
	
	// introduced in version 4.0.0
	
	is >> EncodedString(name, version.codepage());
	
	is >> EncodedString(languageName, (version >= INNO_VERSION(4, 2, 2)) ? 1200 : 1252);
	
	is >> EncodedString(dialogFontName, version.codepage());
	is >> EncodedString(titleFontName, version.codepage());
	is >> EncodedString(welcomeFontName, version.codepage());
	is >> EncodedString(copyrightFontName, version.codepage());
	
	is >> BinaryString(data);
	
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> AnsiString(licenseText);
		is >> AnsiString(infoBeforeText);
		is >> AnsiString(infoAfterText);
	} else {
		licenseText.clear(), infoBeforeText.clear(), infoAfterText.clear();
	}
	
	languageId = loadNumber<u32>(is);
	
	if(version >= INNO_VERSION(4, 2, 2) && (version < INNO_VERSION(5, 3, 0) || !version.unicode)) {
		codepage = loadNumber<u32>(is);
	} else {
		codepage = 0;
	}
	if(!codepage) {
		codepage = version.codepage();
	}
	
	dialogFontSize = loadNumber<u32>(is);
	
	if(version < INNO_VERSION(4, 1, 0)) {
		dialogFontStandardHeight = loadNumber<u32>(is);
	} else {
		dialogFontStandardHeight = 0;
	}
	
	titleFontSize = loadNumber<u32>(is);
	welcomeFontSize = loadNumber<u32>(is);
	copyrightFontSize = loadNumber<u32>(is);
	
	if(version >= INNO_VERSION(5, 2, 3)) {
		rightToLeft = ::load<u8>(is);
	} else {
		rightToLeft = false;
	}
	
}

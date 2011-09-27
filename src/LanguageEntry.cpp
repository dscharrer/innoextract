
#include "LanguageEntry.hpp"

#include <sstream>
#include <iconv.h>
#include "LoadingUtils.hpp"
#include "Output.hpp"

void convert(iconv_t converter, const std::string & from, std::string & to);

void LanguageEntry::load(std::istream & is, const InnoVersion & version) {
	
	// introduced in version 4.0.0
	
	is >> WideString(name, version.unicode);
	
	is >> WideString(languageName, version >= INNO_VERSION(4, 2, 2));
	
	is >> WideString(dialogFontName, version.unicode);
	is >> WideString(titleFontName, version.unicode);
	is >> WideString(welcomeFontName, version.unicode);
	is >> WideString(copyrightFontName, version.unicode);
	
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
		languageCodePage = loadNumber<u32>(is);
	} else {
		languageCodePage = 0;
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

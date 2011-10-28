
#include "setup/LanguageEntry.hpp"

#include <sstream>
#include <iconv.h>

#include "util/LoadingUtils.hpp"
#include "util/Output.hpp"

void convert(iconv_t converter, const std::string & from, std::string & to);

void LanguageEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		is >> EncodedString(name, version.codepage());
	} else {
		name = "default";
	}
	
	is >> EncodedString(languageName, (version >= INNO_VERSION(4, 2, 2)) ? 1200 : 1252);
	
	is >> EncodedString(dialogFontName, version.codepage());
	is >> EncodedString(titleFontName, version.codepage());
	is >> EncodedString(welcomeFontName, version.codepage());
	is >> EncodedString(copyrightFontName, version.codepage());
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		is >> BinaryString(data);
	}
	
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> AnsiString(licenseText);
		is >> AnsiString(infoBeforeText);
		is >> AnsiString(infoAfterText);
	} else {
		licenseText.clear(), infoBeforeText.clear(), infoAfterText.clear();
	}
	
	languageId = loadNumber<uint32_t>(is);
	
	if(version >= INNO_VERSION(4, 2, 2) && (version < INNO_VERSION(5, 3, 0) || !version.unicode)) {
		codepage = loadNumber<uint32_t>(is);
	} else {
		codepage = 0;
	}
	if(!codepage) {
		codepage = version.codepage();
	}
	
	dialogFontSize = loadNumber<uint32_t>(is);
	
	if(version < INNO_VERSION(4, 1, 0)) {
		dialogFontStandardHeight = loadNumber<uint32_t>(is);
	} else {
		dialogFontStandardHeight = 0;
	}
	
	titleFontSize = loadNumber<uint32_t>(is);
	welcomeFontSize = loadNumber<uint32_t>(is);
	copyrightFontSize = loadNumber<uint32_t>(is);
	
	if(version >= INNO_VERSION(5, 2, 3)) {
		rightToLeft = ::load<uint8_t>(is);
	} else {
		rightToLeft = false;
	}
	
}

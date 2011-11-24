
#include "setup/LanguageEntry.hpp"

#include <sstream>
#include <iconv.h>

#include "util/load.hpp"

void convert(iconv_t converter, const std::string & from, std::string & to);

void LanguageEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		is >> encoded_string(name, version.codepage());
	} else {
		name = "default";
	}
	
	is >> encoded_string(languageName, (version >= INNO_VERSION(4, 2, 2)) ? 1200 : 1252);
	
	is >> encoded_string(dialogFontName, version.codepage());
	is >> encoded_string(titleFontName, version.codepage());
	is >> encoded_string(welcomeFontName, version.codepage());
	is >> encoded_string(copyrightFontName, version.codepage());
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		is >> binary_string(data);
	}
	
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> ansi_string(licenseText);
		is >> ansi_string(infoBeforeText);
		is >> ansi_string(infoAfterText);
	} else {
		licenseText.clear(), infoBeforeText.clear(), infoAfterText.clear();
	}
	
	languageId = load_number<uint32_t>(is);
	
	if(version >= INNO_VERSION(4, 2, 2) && (version < INNO_VERSION(5, 3, 0) || !version.unicode)) {
		codepage = load_number<uint32_t>(is);
	} else {
		codepage = 0;
	}
	if(!codepage) {
		codepage = version.codepage();
	}
	
	dialogFontSize = load_number<uint32_t>(is);
	
	if(version < INNO_VERSION(4, 1, 0)) {
		dialogFontStandardHeight = load_number<uint32_t>(is);
	} else {
		dialogFontStandardHeight = 0;
	}
	
	titleFontSize = load_number<uint32_t>(is);
	welcomeFontSize = load_number<uint32_t>(is);
	copyrightFontSize = load_number<uint32_t>(is);
	
	if(version >= INNO_VERSION(5, 2, 3)) {
		rightToLeft = ::load<uint8_t>(is);
	} else {
		rightToLeft = false;
	}
	
}

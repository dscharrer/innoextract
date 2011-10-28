
#ifndef INNOEXTRACT_SETUP_LANGUAGEENTRY_HPP
#define INNOEXTRACT_SETUP_LANGUAGEENTRY_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/Version.hpp"

struct LanguageEntry {
	
	// introduced in 2.0.1
	
	std::string name;
	std::string languageName;
	std::string dialogFontName;
	std::string titleFontName;
	std::string welcomeFontName;
	std::string copyrightFontName;
	std::string data;
	std::string licenseText;
	std::string infoBeforeText;
	std::string infoAfterText;
	
	uint32_t languageId;
	uint32_t codepage;
	size_t dialogFontSize;
	size_t dialogFontStandardHeight;
	size_t titleFontSize;
	size_t welcomeFontSize;
	size_t copyrightFontSize;
	
	bool rightToLeft;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_SETUP_LANGUAGEENTRY_HPP

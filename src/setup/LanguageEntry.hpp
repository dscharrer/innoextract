
#ifndef INNOEXTRACT_SETUP_LANGUAGEENTRY_HPP
#define INNOEXTRACT_SETUP_LANGUAGEENTRY_HPP

#include <iostream>

#include "setup/Version.hpp"
#include "util/Types.hpp"

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
	
	u32 languageId;
	u32 codepage;
	size_t dialogFontSize;
	size_t dialogFontStandardHeight;
	size_t titleFontSize;
	size_t welcomeFontSize;
	size_t copyrightFontSize;
	
	bool rightToLeft;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_SETUP_LANGUAGEENTRY_HPP


#ifndef INNOEXTRACT_LANGUAGEENTRY_HPP
#define INNOEXTRACT_LANGUAGEENTRY_HPP

#include <iostream>
#include "Version.hpp"

struct LanguageEntry {
	
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
	u32 languageCodePage;
	size_t dialogFontSize;
	size_t dialogFontStandardHeight;
	size_t titleFontSize;
	size_t welcomeFontSize;
	size_t copyrightFontSize;
	
	bool rightToLeft;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_LANGUAGEENTRY_HPP


#ifndef INNOEXTRACT_SETUP_ITEM_HPP
#define INNOEXTRACT_SETUP_ITEM_HPP

#include <string>
#include <iosfwd>

#include "setup/windows.hpp"

namespace setup {

struct version;

struct item {
	
	std::string components;
	std::string tasks;
	std::string languages;
	std::string check;
	
	std::string after_install;
	std::string before_install;
	
	windows_version_range winver;
	
protected:
	
	void load_condition_data(std::istream & is, const version & version);
	
	inline void load_version_data(std::istream & is, const version & version) {
		winver.load(is, version);
	}
	
};

} // namespace setup

#endif // INNOEXTRACT_SETUP_ITEM_HPP

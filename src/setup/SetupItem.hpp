
#ifndef INNOEXTRACT_SETUP_SETUPCONDITION_HPP
#define INNOEXTRACT_SETUP_SETUPCONDITION_HPP

#include <string>
#include <iosfwd>

#include "setup/version.hpp"
#include "setup/WindowsVersion.hpp"

struct SetupItem {
	
	std::string components;
	std::string tasks;
	std::string languages;
	std::string check;
	
	std::string afterInstall;
	std::string beforeInstall;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
protected:
	
	void load_condition_data(std::istream & is, const inno_version & version);
	
	void load_version_data(std::istream & is, const inno_version & version);
	
};

#endif // INNOEXTRACT_SETUP_SETUPCONDITION_HPP

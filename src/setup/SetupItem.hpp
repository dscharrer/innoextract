
#ifndef INNOEXTRACT_SETUP_SETUPCONDITION_HPP
#define INNOEXTRACT_SETUP_SETUPCONDITION_HPP

#include <string>
#include <iosfwd>

#include "setup/Version.hpp"
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
	
	void loadConditionData(std::istream & is, const InnoVersion & version);
	
	void loadVersionData(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_SETUP_SETUPCONDITION_HPP


#ifndef INNOEXTRACT_SETUP_SETUPCONDITION_HPP
#define INNOEXTRACT_SETUP_SETUPCONDITION_HPP

#include <iostream>

#include "setup/Version.hpp"

struct SetupCondition {
	
	std::string components;
	std::string tasks;
	std::string languages;
	std::string check;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

struct SetupTasks {
	
	std::string afterInstall;
	std::string beforeInstall;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_SETUP_SETUPCONDITION_HPP


#include "setup/SetupItem.hpp"

#include "util/load.hpp"

void SetupItem::load_condition_data(std::istream & is, const inno_version & version) {
	
	if(version >= INNO_VERSION(2, 0, 0)) {
		is >> encoded_string(components, version.codepage());
		is >> encoded_string(tasks, version.codepage());
	} else {
		components.clear(), tasks.clear();
	}
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> encoded_string(languages, version.codepage());
	} else {
		languages.clear();
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		is >> encoded_string(check, version.codepage());
	} else {
		check.clear();
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		is >> encoded_string(afterInstall, version.codepage());
		is >> encoded_string(beforeInstall, version.codepage());
	} else {
		afterInstall.clear(), beforeInstall.clear();
	}
	
}

void SetupItem::load_version_data(std::istream & is, const inno_version & version) {
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
}

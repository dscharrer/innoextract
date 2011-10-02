
#include "setup/SetupCondition.hpp"

#include "util/LoadingUtils.hpp"

void SetupCondition::load(std::istream & is, const InnoVersion & version) {
	
	if(version > INNO_VERSION(1, 3, 26)) {
		is >> EncodedString(components, version.codepage());
		is >> EncodedString(tasks, version.codepage());
	} else {
		components.clear(), tasks.clear();
	}
	if(version >= INNO_VERSION(4, 0, 1)) {
		is >> EncodedString(languages, version.codepage());
	} else {
		languages.clear();
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		is >> EncodedString(check, version.codepage());
	} else {
		check.clear();
	}
	
}

void SetupTasks::load(std::istream & is, const InnoVersion & version) {
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		is >> EncodedString(afterInstall, version.codepage());
		is >> EncodedString(beforeInstall, version.codepage());
	} else {
		afterInstall.clear(), beforeInstall.clear();
	}
	
}

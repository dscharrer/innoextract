
#ifndef INNOEXTRACT_SETUP_DELETEENTRY_HPP
#define INNOEXTRACT_SETUP_DELETEENTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/Version.hpp"
#include "util/Enum.hpp"

struct DeleteEntry : public SetupItem {
	
	enum Type {
		Files,
		FilesAndSubdirs,
		DirIfEmpty,
	};
	
	std::string name;
	
	Type type;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

NAMED_ENUM(DeleteEntry::Type)

#endif // INNOEXTRACT_SETUP_DELETEENTRY_HPP

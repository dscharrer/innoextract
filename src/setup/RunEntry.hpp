
#ifndef INNOEXTRACT_SETUP_RUNENTRY_HPP
#define INNOEXTRACT_SETUP_RUNENTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/Version.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

struct RunEntry : public SetupItem {
	
	FLAGS(Options,
		ShellExec,
		SkipIfDoesntExist,
		PostInstall,
		Unchecked,
		SkipIfSilent,
		Skipif_not_equalSilent,
		HideWizard,
		Bits32,
		Bits64,
		RunAsOriginalUser
	);
	
	enum Wait {
		WaitUntilTerminated,
		NoWait,
		WaitUntilIdle,
	};
	
	std::string name;
	std::string parameters;
	std::string workingDir;
	std::string runOnceId;
	std::string statusMessage;
	std::string verb;
	std::string description;
	
	int showCmd;
	
	Wait wait;
	
	Options options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(RunEntry::Options)
NAMED_ENUM(RunEntry::Options)

NAMED_ENUM(RunEntry::Wait)

#endif // INNOEXTRACT_SETUP_RUNENTRY_HPP

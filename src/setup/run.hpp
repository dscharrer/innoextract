
#ifndef INNOEXTRACT_SETUP_RUN_HPP
#define INNOEXTRACT_SETUP_RUN_HPP

#include <string>
#include <iosfwd>

#include "setup/item.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

struct run_entry : public item {
	
	FLAGS(flags,
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
	
	enum wait_condition {
		WaitUntilTerminated,
		NoWait,
		WaitUntilIdle,
	};
	
	std::string name;
	std::string parameters;
	std::string working_dir;
	std::string run_once_id;
	std::string status_message;
	std::string verb;
	std::string description;
	
	int show_command;
	
	wait_condition wait;
	
	flags options;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_FLAGS(setup::run_entry::flags)
NAMED_ENUM(setup::run_entry::wait_condition)

#endif // INNOEXTRACT_SETUP_RUN_HPP

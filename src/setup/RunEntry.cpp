
#include "setup/RunEntry.hpp"

#include <stdint.h>

#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace {

STORED_ENUM_MAP(StoredRunWait, RunEntry::WaitUntilTerminated,
	RunEntry::WaitUntilTerminated,
	RunEntry::NoWait,
	RunEntry::WaitUntilIdle,
);

} // anonymous namespace

void RunEntry::load(std::istream & is, const inno_version & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the directory entry structure
	}
	
	is >> encoded_string(name, version.codepage());
	is >> encoded_string(parameters, version.codepage());
	is >> encoded_string(workingDir, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> encoded_string(runOnceId, version.codepage());
	} else {
		runOnceId.clear();
	}
	if(version >= INNO_VERSION(2, 0, 2)) {
		is >> encoded_string(statusMessage, version.codepage());
	} else {
		statusMessage.clear();
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		is >> encoded_string(verb, version.codepage());
	} else {
		verb.clear();
	}
	if(version >= INNO_VERSION(2, 0, 0)) {
		is >> encoded_string(description, version.codepage());
	}
	
	load_condition_data(is, version);
	
	load_version_data(is, version);
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		showCmd = load_number<int32_t>(is);
	} else {
		showCmd = 0;
	}
	
	wait = stored_enum<StoredRunWait>(is).get();
	
	stored_flag_reader<Options> flags(is);
	
	flags.add(ShellExec);
	if(version >= INNO_VERSION(1, 3, 21)) {
		flags.add(SkipIfDoesntExist);
	}
	if(version >= INNO_VERSION(2, 0, 0)) {
		flags.add(PostInstall);
		flags.add(Unchecked);
		flags.add(SkipIfSilent);
		flags.add(Skipif_not_equalSilent);
	}
	if(version >= INNO_VERSION(2, 0, 8)) {
		flags.add(HideWizard);
	}
	if(version >= INNO_VERSION(5, 1, 10)) {
		flags.add(Bits32);
		flags.add(Bits64);
	}
	if(version >= INNO_VERSION(5, 2, 0)) {
		flags.add(RunAsOriginalUser);
	}
	
	options = flags;
}

ENUM_NAMES(RunEntry::Options, "Run Option",
	"shell exec",
	"skip if doesn't exist",
	"post install",
	"unchecked",
	"skip if silent",
	"skip if not silent",
	"hide wizard",
	"32 bit",
	"64 bit",
	"run as original user",
)

ENUM_NAMES(RunEntry::Wait, "Run Wait Type",
	"wait until terminated",
	"no wait",
	"wait until idle",
)

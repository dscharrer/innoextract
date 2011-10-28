
#include "setup/RunEntry.hpp"

#include <stdint.h>

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

namespace {

STORED_ENUM_MAP(StoredRunWait, RunEntry::WaitUntilTerminated,
	RunEntry::WaitUntilTerminated,
	RunEntry::NoWait,
	RunEntry::WaitUntilIdle,
);

} // anonymous namespace

void RunEntry::load(std::istream & is, const InnoVersion & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the directory entry structure
	}
	
	is >> EncodedString(name, version.codepage());
	is >> EncodedString(parameters, version.codepage());
	is >> EncodedString(workingDir, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> EncodedString(runOnceId, version.codepage());
	} else {
		runOnceId.clear();
	}
	if(version >= INNO_VERSION(2, 0, 2)) {
		is >> EncodedString(statusMessage, version.codepage());
	} else {
		statusMessage.clear();
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		is >> EncodedString(verb, version.codepage());
	} else {
		verb.clear();
	}
	if(version >= INNO_VERSION(2, 0, 0)) {
		is >> EncodedString(description, version.codepage());
	}
	
	loadConditionData(is, version);
	
	loadVersionData(is, version);
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		showCmd = loadNumber<int32_t>(is);
	} else {
		showCmd = 0;
	}
	
	wait = StoredEnum<StoredRunWait>(is).get();
	
	StoredFlagReader<Options> flags(is);
	
	flags.add(ShellExec);
	if(version >= INNO_VERSION(1, 3, 21)) {
		flags.add(SkipIfDoesntExist);
	}
	if(version >= INNO_VERSION(2, 0, 0)) {
		flags.add(PostInstall);
		flags.add(Unchecked);
		flags.add(SkipIfSilent);
		flags.add(SkipIfNotSilent);
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
	
	options = flags.get();
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

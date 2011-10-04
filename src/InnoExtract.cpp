
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <bitset>
#include <ctime>

#include <boost/shared_ptr.hpp>

#include "loader/SetupLoader.hpp"

#include "setup/MessageEntry.hpp"
#include "setup/DeleteEntry.hpp"
#include "setup/DirectoryEntry.hpp"
#include "setup/FileEntry.hpp"
#include "setup/FileLocationEntry.hpp"
#include "setup/IconEntry.hpp"
#include "setup/IniEntry.hpp"
#include "setup/LanguageEntry.hpp"
#include "setup/PermissionEntry.hpp"
#include "setup/RegistryEntry.hpp"
#include "setup/RunEntry.hpp"
#include "setup/SetupComponentEntry.hpp"
#include "setup/SetupHeader.hpp"
#include "setup/SetupTaskEntry.hpp"
#include "setup/SetupTypeEntry.hpp"
#include "setup/Version.hpp"

#include "stream/BlockReader.hpp"

#include "util/LoadingUtils.hpp"
#include "util/Output.hpp"
#include "util/Utils.hpp"

using std::cout;
using std::string;
using std::endl;
using std::setw;
using std::setfill;
using std::hex;
using std::dec;

void printSetupItem(std::ostream & os, const SetupItem & item, const SetupHeader & header) {
	
	os << IfNotEmpty("  Componenets", item.components);
	os << IfNotEmpty("  Tasks", item.tasks);
	os << IfNotEmpty("  Languages", item.languages);
	os << IfNotEmpty("  Check", item.check);
	
	os << IfNotEmpty("  After install", item.afterInstall);
	os << IfNotEmpty("  Before install", item.beforeInstall);
	
	os << IfNot("  Min version", item.minVersion, header.minVersion);
	os << IfNot("  Only below version", item.onlyBelowVersion, header.onlyBelowVersion);
	
}

void print(std::ostream & os, const RunEntry & entry, const SetupHeader & header) {
	
	os << " - " << Quoted(entry.name) << ':' << endl;
	os << IfNotEmpty("  Parameters", entry.parameters);
	os << IfNotEmpty("  Working directory", entry.workingDir);
	os << IfNotEmpty("  Run once id", entry.runOnceId);
	os << IfNotEmpty("  Status message", entry.statusMessage);
	os << IfNotEmpty("  Verb", entry.verb);
	os << IfNotEmpty("  Description", entry.verb);
	
	printSetupItem(cout, entry, header);
	
	os << IfNot("  Show command", entry.showCmd, 1);
	os << IfNot("  Wait", entry.wait, RunEntry::WaitUntilTerminated);
	
	os << IfNotZero("  Options", entry.options);
	
}

std::ostream & operator<<(std::ostream & os, const Checksum & checksum) {
	
	std::ios_base::fmtflags old = os.flags();
	os << std::hex;
	
	cout << checksum.type << ' ';
	
	switch(checksum.type) {
		case Checksum::Adler32: {
			cout << "0x" << std::setfill('0') << std::setw(8) << checksum.adler32;
			break;
		}
		case Checksum::Crc32: {
			cout << "0x" << std::setfill('0') << std::setw(8) << checksum.crc32;
			break;
		}
		case Checksum::MD5: {
			for(size_t i = 0; i < ARRAY_SIZE(checksum.md5); i++) {
				cout << std::setfill('0') << std::setw(0) << int(u8(checksum.md5[i]));
			}
		}
		case Checksum::Sha1: {
			for(size_t i = 0; i < ARRAY_SIZE(checksum.sha1); i++) {
				cout << std::setfill('0') << std::setw(0) << int(u8(checksum.sha1[i]));
			}
		}
	}
	
	os.setf(old, std::ios_base::basefield);
	
	return os;
}

static const char * magicNumbers[][2] = {
	{ "GIF89a", "gif" },
	{ "GIF87a", "gif" },
	{ "\xFF\xD8", "jpg" },
	{ "\x89PNG\r\n\x1A\n", "png" },
	{ "%PDF", "pdf" },
	{ "MZ", "dll" },
	{ "BM", "bmp" },
};

const char * guessExtension(const string & data) {
	
	for(size_t i = 0; i < ARRAY_SIZE(magicNumbers); i++) {
		
		size_t n = strlen(magicNumbers[i][0]);
		
		if(!data.compare(0, n, magicNumbers[i][0], n)) {
			return magicNumbers[i][1];
		}
	}
	
	return "bin";
}

void dump(std::istream & is, const string & file) {
	
	// TODO stream
	
	std::string data;
	is >> BinaryString(data);
	cout << "Resource: " << color::cyan << file << color::reset << ": " << color::white << data.length() << color::reset << " bytes" << endl;
	
	if(data.empty()) {
		return;
	}
	
	std::string filename = file + '.' + guessExtension(data);
	
	std::ofstream ofs(filename.c_str(), strm::trunc | strm::binary | strm::out);
	
	ofs << data;
};

void readWizardImageAndDecompressor(std::istream & is, const InnoVersion & version, const SetupHeader & header) {
	
	cout << endl;
	
	dump(is, "wizard");
	
	if(version >= INNO_VERSION(2, 0, 0)) {
		dump(is, "wizard_small");
	}
	
	if(header.compressMethod == SetupHeader::BZip2
			|| (header.compressMethod == SetupHeader::LZMA1 && version == INNO_VERSION(4, 1, 5))
			|| (header.compressMethod == SetupHeader::Zlib && version >= INNO_VERSION(4, 2, 6))) {
		
		dump(is, "decompressor");
	}
	
	if(is.fail()) {
		error << "error reading misc setup data";
	}
	
}

int main(int argc, char * argv[]) {
	
	if(argc <= 1) {
		std::cout << "usage: innoextract <Inno Setup installer>" << endl;
		return 1;
	}
	
	std::ifstream ifs(argv[1], strm::in | strm::binary | strm::ate);
	
	if(!ifs.is_open()) {
		error << "error opening file";
		return 1;
	}
	
	u64 fileSize = ifs.tellg();
	if(!fileSize) {
		error << "cannot read file or empty file";
		return 1;
	}
	
	SetupLoader::Offsets offsets;
	if(!SetupLoader::getOffsets(ifs, offsets)) {
		error << "failed to load setup loader offsets";
		// TODO try offset0 = 0
		return 1;
	}
	
	cout << std::boolalpha;
	
	cout << color::white;
	cout << "loaded offsets:" << endl;
	cout << "- total size: " << offsets.totalSize << endl;
	cout << "- exe: @ " << hex << offsets.exeOffset << dec << "  compressed: " << offsets.exeCompressedSize << "  uncompressed: " << offsets.exeUncompressedSize << endl;
	cout << "- exe checksum: " << hex << setfill('0') << setw(8) << offsets.exeChecksum << dec << " (" << (offsets.exeChecksumMode == ChecksumAdler32 ? "Alder32" : "CRC32") << ')' << endl;
	cout << "- messageOffset: " << hex << offsets.messageOffset << dec << endl;
	cout << "- offset:  0: " << hex << offsets.offset0 << "  1: " << offsets.messageOffset << dec << endl;
	cout << color::reset;
	
	ifs.seekg(offsets.offset0);
	
	InnoVersion version;
	version.load(ifs);
	if(ifs.fail()) {
		error << "error reading setup data version!";
		return 1;
	}
	
	cout << "version: " << color::white << version << color::reset << endl;
	
	boost::shared_ptr<std::istream> is(BlockReader::get(ifs, version));
	if(!is) {
		error << "error reading block";
		return 1;
	}
	
	is->exceptions(strm::badbit | strm::failbit);
	
	SetupHeader header;
	header.load(*is, version);
	if(is->fail()) {
		error << "error reading setup data header!";
		return 1;
	}
	
	cout << endl;
	
	cout << IfNotEmpty("App name", header.appName);
	cout << IfNotEmpty("App ver name", header.appVerName);
	cout << IfNotEmpty("App id", header.appId);
	cout << IfNotEmpty("Copyright", header.appCopyright);
	cout << IfNotEmpty("Publisher", header.appPublisher);
	cout << IfNotEmpty("Publisher URL", header.appPublisherURL);
	cout << IfNotEmpty("Support phone", header.appSupportPhone);
	cout << IfNotEmpty("Support URL", header.appSupportURL);
	cout << IfNotEmpty("Updates URL", header.appUpdatesURL);
	cout << IfNotEmpty("Version", header.appVersion);
	cout << IfNotEmpty("Default dir name", header.defaultDirName);
	cout << IfNotEmpty("Default group name", header.defaultGroupName);
	cout << IfNotEmpty("Uninstall icon name", header.uninstallIconName);
	cout << IfNotEmpty("Base filename", header.baseFilename);
	cout << IfNotEmpty("Uninstall files dir", header.uninstallFilesDir);
	cout << IfNotEmpty("Uninstall display name", header.uninstallDisplayName);
	cout << IfNotEmpty("Uninstall display icon", header.uninstallDisplayIcon);
	cout << IfNotEmpty("App mutex", header.appMutex);
	cout << IfNotEmpty("Default user name", header.defaultUserInfoName);
	cout << IfNotEmpty("Default user org", header.defaultUserInfoOrg);
	cout << IfNotEmpty("Default user serial", header.defaultUserInfoSerial);
	cout << IfNotEmpty("Readme", header.appReadmeFile);
	cout << IfNotEmpty("Contact", header.appContact);
	cout << IfNotEmpty("Comments", header.appComments);
	cout << IfNotEmpty("Modify path", header.appModifyPath);
	cout << IfNotEmpty("Uninstall reg key", header.createUninstallRegKey);
	cout << IfNotEmpty("Uninstallable", header.uninstallable);
	cout << IfNotEmpty("License", header.licenseText);
	cout << IfNotEmpty("Info before text", header.infoBeforeText);
	cout << IfNotEmpty("Info after text", header.infoAfterText);
	cout << IfNotEmpty("Uninstaller signature", header.signedUninstallerSignature);
	cout << IfNotEmpty("Compiled code", header.compiledCodeText);
	
	cout << IfNotZero("Lead bytes", header.leadBytes);
	
	cout << IfNotZero("Language entries", header.numLanguageEntries);
	cout << IfNotZero("Custom message entries", header.numCustomMessageEntries);
	cout << IfNotZero("Permission entries", header.numPermissionEntries);
	cout << IfNotZero("Type entries", header.numTypeEntries);
	cout << IfNotZero("Component entries", header.numComponentEntries);
	cout << IfNotZero("Task entries", header.numTaskEntries);
	cout << IfNotZero("Dir entries", header.numDirectoryEntries);
	cout << IfNotZero("File entries", header.numFileEntries);
	cout << IfNotZero("File location entries", header.numFileLocationEntries);
	cout << IfNotZero("Icon entries", header.numIconEntries);
	cout << IfNotZero("Ini entries", header.numIniEntries);
	cout << IfNotZero("Registry entries", header.numRegistryEntries);
	cout << IfNotZero("Delete entries", header.numDeleteEntries);
	cout << IfNotZero("Uninstall delete entries", header.numUninstallDeleteEntries);
	cout << IfNotZero("Run entries", header.numRunEntries);
	cout << IfNotZero("Uninstall run entries", header.numUninstallRunEntries);
	
	cout << IfNot("Min version", header.minVersion, WindowsVersion::none);
	cout << IfNot("Only below version", header.onlyBelowVersion, WindowsVersion::none);
	
	cout << hex;
	cout << IfNotZero("Back color", header.backColor);
	cout << IfNotZero("Back color2", header.backColor2);
	cout << IfNotZero("Wizard image back color", header.wizardImageBackColor);
	cout << IfNotZero("Wizard small image back color", header.wizardSmallImageBackColor);
	cout << dec;
	
	if(header.options & (SetupHeader::Password | SetupHeader::EncryptionUsed)) {
		cout << "Password: " << color::cyan << header.password << color::reset << endl;
		// TODO print salt
	}
	
	cout << IfNotZero("Extra disk space required", header.extraDiskSpaceRequired);
	cout << IfNotZero("Slices per disk", header.slicesPerDisk);
	
	cout << IfNot("Install mode", header.installMode, SetupHeader::NormalInstallMode);
	cout << "Uninstall log mode: " << color::cyan << header.uninstallLogMode << color::reset << endl;
	cout << "Uninstall style: " << color::cyan << header.uninstallStyle << color::reset << endl;
	cout << "Dir exists warning: " << color::cyan << header.dirExistsWarning << color::reset << endl;
	cout << IfNot("Privileges required", header.privilegesRequired, SetupHeader::NoPrivileges);
	cout << "Show language dialog: " << color::cyan << header.showLanguageDialog << color::reset << endl;
	cout << IfNot("Danguage detection", header.languageDetectionMethod, SetupHeader::NoLanguageDetection);
	cout << "Compression: " << color::cyan << header.compressMethod << color::reset << endl;
	cout << "Architectures allowed: " << color::cyan << header.architecturesAllowed << color::reset << endl;
	cout << "Architectures installed in 64-bit mode: " << color::cyan << header.architecturesInstallIn64BitMode << color::reset << endl;
	
	if(header.options & SetupHeader::SignedUninstaller) {
		cout << IfNotZero("Size before signing uninstaller", header.signedUninstallerOrigSize);
		cout << IfNotZero("Uninstaller header checksum", header.signedUninstallerHdrChecksum);
	}
	
	cout << "Disable dir page: " << color::cyan << header.disableDirPage << color::reset << endl;
	cout << "Disable program group page: " << color::cyan << header.disableProgramGroupPage << color::reset << endl;
	
	cout << IfNotZero("Uninstall display size", header.uninstallDisplaySize);
	
	cout << "Options: " << color::green << header.options << color::reset << endl;
	
	cout << color::reset;
	
	if(header.numLanguageEntries) {
		cout << endl << "Language entries:" << endl;
	}
	std::vector<LanguageEntry> languages;
	languages.resize(header.numLanguageEntries);
	for(size_t i = 0; i < header.numLanguageEntries; i++) {
		
		LanguageEntry & entry = languages[i];
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading language entry #" << i;
		}
		
		cout << " - " << Quoted(entry.name) << ':' << endl;
		cout << IfNotEmpty("  Language name", entry.languageName);
		cout << IfNotEmpty("  Dialog font", entry.dialogFontName);
		cout << IfNotEmpty("  Title font", entry.titleFontName);
		cout << IfNotEmpty("  Welcome font", entry.welcomeFontName);
		cout << IfNotEmpty("  Copyright font", entry.copyrightFontName);
		cout << IfNotEmpty("  Data", entry.data);
		cout << IfNotEmpty("  License", entry.licenseText);
		cout << IfNotEmpty("  Info before text", entry.infoBeforeText);
		cout << IfNotEmpty("  Info after text", entry.infoAfterText);
		
		cout << "  Language id: " << color::cyan << hex << entry.languageId << dec << color::reset << endl;
		
		cout << IfNotZero("  Codepage", entry.codepage);
		cout << IfNotZero("  Dialog font size", entry.dialogFontSize);
		cout << IfNotZero("  Dialog font standard height", entry.dialogFontStandardHeight);
		cout << IfNotZero("  Title font size", entry.titleFontSize);
		cout << IfNotZero("  Welcome font size", entry.welcomeFontSize);
		cout << IfNotZero("  Copyright font size", entry.copyrightFontSize);
		cout << IfNot("  Right to left", entry.rightToLeft, false);
		
	};
	
	if(version < INNO_VERSION(4, 0, 0)) {
		readWizardImageAndDecompressor(*is, version, header);
	}
	
	if(header.numCustomMessageEntries) {
		cout << endl << "Message entries:" << endl;
	}
	for(size_t i = 0; i < header.numCustomMessageEntries; i++) {
		
		MessageEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading custom message entry #" << i;
		}
		
		if(entry.language >= 0 ? size_t(entry.language) >= languages.size() : entry.language != -1) {
			warning << "unexpected language index: " << entry.language;
		}
		
		int codepage;
		if(entry.language == -1) {
			codepage = version.codepage();
		} else {
			codepage = languages[entry.language].codepage;
		}
		
		string decoded;
		toUtf8(entry.value, decoded, codepage);
		
		cout << " - " << Quoted(entry.name);
		if(entry.language == -1) {
			cout << " (default) = ";
		} else {
			cout << " (" << color::cyan << languages[entry.language].name << color::reset << ") = ";
		}
		cout << Quoted(decoded) << endl;
		
	}
	
	if(header.numPermissionEntries) {
		cout << endl << "Permission entries:" << endl;
	}
	for(size_t i = 0; i < header.numPermissionEntries; i++) {
		
		PermissionEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading permission entry #" << i;
		}
		
		cout << " - " << entry.permissions.length() << " bytes";
		
	}
	
	if(header.numTypeEntries) {
		cout << endl << "Type entries:" << endl;
	}
	for(size_t i = 0; i < header.numTypeEntries; i++) {
		
		SetupTypeEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading type entry #" << i;
		}
		
		cout << " - " << Quoted(entry.name) << ':' << endl;
		cout << IfNotEmpty("  Description", entry.description);
		cout << IfNotEmpty("  Languages", entry.languages);
		cout << IfNotEmpty("  Check", entry.check);
		
		cout << IfNot("  Min version", entry.minVersion, header.minVersion);
		cout << IfNot("  Only below version", entry.onlyBelowVersion, header.onlyBelowVersion);
		
		cout << IfNotZero("  Options", entry.options);
		cout << IfNot("  Type", entry.type, SetupTypeEntry::User);
		cout << IfNotZero("  Size", entry.size);
		
	}
	
	if(header.numComponentEntries) {
		cout << endl << "Component entries:" << endl;
	}
	for(size_t i = 0; i < header.numComponentEntries; i++) {
		
		SetupComponentEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading component entry #" << i;
		}
		
		cout << " - " << Quoted(entry.name) << ':' << endl;
		cout << IfNotEmpty("  Types", entry.types);
		cout << IfNotEmpty("  Description", entry.description);
		cout << IfNotEmpty("  Languages", entry.languages);
		cout << IfNotEmpty("  Check", entry.check);
		
		cout << IfNotZero("  Extra disk space required", entry.extraDiskSpaceRequired);
		cout << IfNotZero("  Level", entry.level);
		cout << IfNot("  Used", entry.used, true);
		
		cout << IfNot("  Min version", entry.minVersion, header.minVersion);
		cout << IfNot("  Only below version", entry.onlyBelowVersion, header.onlyBelowVersion);
		
		cout << IfNotZero("  Options", entry.options);
		cout << IfNotZero("  Size", entry.size);
		
	}
	
	if(header.numTaskEntries) {
		cout << endl << "Task entries:" << endl;
	}
	for(size_t i = 0; i < header.numTaskEntries; i++) {
		
		SetupTaskEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading task entry #" << i;
		}
		
		cout << " - " << Quoted(entry.name) << ':' << endl;
		cout << IfNotEmpty("  Description", entry.description);
		cout << IfNotEmpty("  Group description", entry.groupDescription);
		cout << IfNotEmpty("  Components", entry.components);
		cout << IfNotEmpty("  Languages", entry.languages);
		cout << IfNotEmpty("  Check", entry.check);
		
		cout << IfNotZero("  Level", entry.level);
		cout << IfNot("  Used", entry.used, true);
		
		cout << IfNot("  Min version", entry.minVersion, header.minVersion);
		cout << IfNot("  Only below version", entry.onlyBelowVersion, header.onlyBelowVersion);
		
		cout << IfNotZero("  Options", entry.options);
		
	}
	
	if(header.numDirectoryEntries) {
		cout << endl << "Directory entries:" << endl;
	}
	for(size_t i = 0; i < header.numDirectoryEntries; i++) {
		
		DirectoryEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading directory entry #" << i;
		}
		
		cout << " - " << Quoted(entry.name) << ':' << endl;
		
		printSetupItem(cout, entry, header);
		
		if(!entry.permissions.empty()) {
			cout << "  Permissions: " << entry.permissions.length() << " bytes";
		}
		
		
		cout << IfNotZero("  Attributes", entry.attributes);
		
		cout << IfNot("  Permission entry", entry.permission, -1);
		
		cout << IfNotZero("  Options", entry.options);
		
	}
	
	if(header.numFileEntries) {
		cout << endl << "File entries:" << endl;
	}
	for(size_t i = 0; i < header.numFileEntries; i++) {
		
		FileEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading file entry #" << i;
		}
		
		if(entry.destination.empty()) {
			cout << " - File #" << i;
		} else {
			cout << " - " << Quoted(entry.destination);
		}
		if(entry.location != -1) {
			cout << " (location: " << color::cyan << entry.location << color::reset << ')';
		}
		cout  << endl;
		
		cout << IfNotEmpty("  Source", entry.source);
		cout << IfNotEmpty("  Install font name", entry.installFontName);
		cout << IfNotEmpty("  Strong assembly name", entry.strongAssemblyName);
		
		printSetupItem(cout, entry, header);
		
		cout << IfNotZero("  Attributes", entry.attributes);
		cout << IfNotZero("  Size", entry.externalSize);
		
		cout << IfNot("  Permission entry", entry.permission, -1);
		
		cout << IfNotZero("  Options", entry.options);
		
		cout << IfNot("  Type", entry.type, FileEntry::UserFile);
		
	}
	
	if(header.numIconEntries) {
		cout << endl << "Icon entries:" << endl;
	}
	for(size_t i = 0; i < header.numIconEntries; i++) {
		
		IconEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading icon entry #" << i;
		}
		
		cout << " - " << Quoted(entry.name) << " -> " << Quoted(entry.filename) << endl;
		cout << IfNotEmpty("  Parameters", entry.parameters);
		cout << IfNotEmpty("  Working directory", entry.workingDir);
		cout << IfNotEmpty("  Icon file", entry.iconFilename);
		cout << IfNotEmpty("  Comment", entry.comment);
		cout << IfNotEmpty("  App user model id", entry.appUserModelId);
		
		printSetupItem(cout, entry, header);
		
		cout << IfNotZero("  Icon index", entry.iconIndex);
		cout << IfNot("  Show command", entry.showCmd, 1);
		cout << IfNot("  Close on exit", entry.closeOnExit, IconEntry::NoSetting);
		
		cout << IfNotZero("  Hotkey", entry.hotkey);
		
		cout << IfNotZero("  Options", entry.options);
		
	}
	
	if(header.numIniEntries) {
		cout << endl << "Ini entries:" << endl;
	}
	for(size_t i = 0; i < header.numIniEntries; i++) {
		
		IniEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading ini entry #" << i;
		}
		
		cout << " - in " << Quoted(entry.inifile);
		cout << " set [" << Quoted(entry.section) << "] ";
		cout << Quoted(entry.key) << " = " << Quoted(entry.value) << std::endl;
		
		printSetupItem(cout, entry, header);
		
		cout << IfNotZero("  Options", entry.options);
		
	}
	
	if(header.numRegistryEntries) {
		cout << endl << "Registry entries:" << endl;
	}
	for(size_t i = 0; i < header.numRegistryEntries; i++) {
		
		RegistryEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading registry entry #" << i;
		}
		
		cout << " - ";
		if(entry.hive != RegistryEntry::Unset) {
			cout << entry.hive << '\\';
		}
		cout << Quoted(entry.key);
		cout << endl << "  ";
		if(entry.name.empty()) {
			cout << "(default)";
		} else {
			cout << Quoted(entry.name);
		}
		if(!entry.value.empty()) {
			cout << " = " << Quoted(entry.value);
		}
		if(entry.type != RegistryEntry::None) {
			cout << " (" << color::cyan << entry.type << color::reset << ')';
		}
		cout << endl;
		
		printSetupItem(cout, entry, header);
		
		if(!entry.permissions.empty()) {
			cout << "  Permissions: " << entry.permissions.length() << " bytes";
		}
		cout << IfNot("  Permission entry", entry.permission, -1);
		
		cout << IfNotZero("  Options", entry.options);
		
	}
	
	if(header.numDeleteEntries) {
		cout << endl << "Delete entries:" << endl;
	}
	for(size_t i = 0; i < header.numDeleteEntries; i++) {
		
		DeleteEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading install delete entry #" << i;
		}
		
		cout << " - " << Quoted(entry.name)
		     << " (" << color::cyan << entry.type << color::reset << ')' << endl;
		
		printSetupItem(cout, entry, header);
		
	}
	
	if(header.numUninstallDeleteEntries) {
		cout << endl << "Uninstall delete entries:" << endl;
	}
	for(size_t i = 0; i < header.numUninstallDeleteEntries; i++) {
		
		DeleteEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading uninstall delete entry #" << i;
		}
		
		cout << " - " << Quoted(entry.name)
		     << " (" << color::cyan << entry.type << color::reset << ')' << endl;
		
		printSetupItem(cout, entry, header);
		
	}
	
	if(header.numRunEntries) {
		cout << endl << "Run entries:" << endl;
	}
	for(size_t i = 0; i < header.numRunEntries; i++) {
		
		RunEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading install run entry #" << i;
		}
		
		print(cout, entry, header);
		
	}
	
	if(header.numUninstallRunEntries) {
		cout << endl << "Uninstall run entries:" << endl;
	}
	for(size_t i = 0; i < header.numUninstallRunEntries; i++) {
		
		RunEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading uninstall run entry #" << i;
		}
		
		print(cout, entry, header);
		
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		readWizardImageAndDecompressor(*is, version, header);
	}
	
	{
		is->exceptions(strm::goodbit);
		char dummy;
		if(!is->get(dummy).eof()) {
			warning << "expected end of stream";
		}
	}
	
	// TODO skip to end if not there yet
	
	is.reset(BlockReader::get(ifs, version));
	if(!is) {
		error << "error reading block";
		return 1;
	}
	
	is->exceptions(strm::badbit | strm::failbit);
	
	if(header.numFileLocationEntries) {
		cout << endl << "File location entries:" << endl;
	}
	for(size_t i = 0; i < header.numFileLocationEntries; i++) {
		
		FileLocationEntry entry;
		entry.load(*is, version);
		if(is->fail()) {
			error << "error reading file location entry #" << i;
		}
		
		cout << " - " << "File location #" << i << ':' << endl;
		
		cout << IfNotZero("  First slice", entry.firstSlice);
		cout << IfNot("  Last slice", entry.lastSlice, entry.firstSlice);
		
		cout << IfNotZero("  Start offset: ", PrintHex(entry.chunkSubOffset));
		
		cout << IfNotZero("  Chunk sub offset", PrintHex(entry.chunkSubOffset));
		
		cout << IfNotZero("  Original size", PrintBytes(entry.originalSize));
		
		cout << IfNotZero("  Chunk compressed size", PrintHex(entry.chunkCompressedSize));
		
		cout << "  Checksum: " << entry.checksum << endl;
		
		std::tm t;
		if(entry.options & FileLocationEntry::TimeStampInUTC) {
			gmtime_r(&entry.timestamp.tv_sec, &t);
		} else {
			localtime_r(&entry.timestamp.tv_sec, &t);
		}
		
		cout << "  Timestamp: " << color::cyan << (t.tm_year + 1900)
		     << '-' << std::setfill('0') << std::setw(2) << (t.tm_mon + 1)
				 << '-' << std::setfill('0') << std::setw(2) << t.tm_mday
				 << ' ' << std::setfill(' ') << std::setw(2) << t.tm_hour
				 << ':' << std::setfill('0') << std::setw(2) << t.tm_min
				 << ':' << std::setfill('0') << std::setw(2) << t.tm_sec
				 << color::reset << " +" << entry.timestamp.tv_nsec << endl;
		
		cout << IfNotZero("  Options", entry.options);
		
		if(entry.options & FileLocationEntry::VersionInfoValid) {
			cout << IfNotZero("  File version LS", entry.fileVersionLS);
			cout << IfNotZero("  File version MS", entry.fileVersionMS);
		}
		
	}
	
	{
		is->exceptions(strm::goodbit);
		char dummy;
		if(!is->get(dummy).eof()) {
			warning << "expected end of stream";
		}
	}
	
	return 0;
}


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
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/ref.hpp>
#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/copy.hpp>

#include "loader/offsets.hpp"

#include "setup/component.hpp"
#include "setup/data.hpp"
#include "setup/delete.hpp"
#include "setup/directory.hpp"
#include "setup/file.hpp"
#include "setup/header.hpp"
#include "setup/icon.hpp"
#include "setup/ini.hpp"
#include "setup/language.hpp"
#include "setup/message.hpp"
#include "setup/permission.hpp"
#include "setup/registry.hpp"
#include "setup/run.hpp"
#include "setup/task.hpp"
#include "setup/type.hpp"
#include "setup/version.hpp"

#include "stream/block.hpp"
#include "stream/chunk.hpp"
#include "stream/file.hpp"
#include "stream/slice.hpp"

#include "util/console.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/output.hpp"

using std::cout;
using std::string;
using std::endl;
using std::setw;
using std::setfill;

namespace io = boost::iostreams;
namespace fs = boost::filesystem;

static void print(std::ostream & os, const setup::windows_version_range & winver,
                  const setup::header & header) {
	
	const setup::windows_version_range & def = header.winver;
	
	os << if_not_equal("  Min version", winver.begin, def.begin);
	os << if_not_equal("  Only below version", winver.end, def.end);
}

static void print(std::ostream & os, const setup::item & item,
                 const setup::header & header) {
	
	os << if_not_empty("  Componenets", item.components);
	os << if_not_empty("  Tasks", item.tasks);
	os << if_not_empty("  Languages", item.languages);
	os << if_not_empty("  Check", item.check);
	
	os << if_not_empty("  After install", item.after_install);
	os << if_not_empty("  Before install", item.before_install);
	
	print(os, item.winver, header);
}

static void print(std::ostream & os, const setup::run_entry & entry, const setup::header & header) {
	
	os << " - " << quoted(entry.name) << ':' << endl;
	os << if_not_empty("  Parameters", entry.parameters);
	os << if_not_empty("  Working directory", entry.working_dir);
	os << if_not_empty("  Run once id", entry.run_once_id);
	os << if_not_empty("  Status message", entry.status_message);
	os << if_not_empty("  Verb", entry.verb);
	os << if_not_empty("  Description", entry.verb);
	
	print(cout, static_cast<const setup::item &>(entry), header);
	
	os << if_not_equal("  Show command", entry.show_command, 1);
	os << if_not_equal("  Wait", entry.wait, setup::run_entry::WaitUntilTerminated);
	
	os << if_not_zero("  Options", entry.options);
	
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

static const char * guessExtension(const string & data) {
	
	for(size_t i = 0; i < ARRAY_SIZE(magicNumbers); i++) {
		
		size_t n = strlen(magicNumbers[i][0]);
		
		if(!data.compare(0, n, magicNumbers[i][0], n)) {
			return magicNumbers[i][1];
		}
	}
	
	return "bin";
}

static void dump(std::istream & is, const string & file) {
	
	// TODO stream
	
	std::string data;
	is >> binary_string(data);
	cout << "Resource: " << color::cyan << file << color::reset << ": " << color::white
	     << data.length() << color::reset << " bytes" << endl;
	
	if(data.empty()) {
		return;
	}
	
	std::string filename = file + '.' + guessExtension(data);
	
	std::ofstream ofs(filename.c_str(), std::ios_base::trunc | std::ios_base::binary
	                                    | std::ios_base::out);
	
	ofs << data;
};

static void readWizardImageAndDecompressor(std::istream & is, const setup::version & version,
                                           const setup::header & header) {
	
	cout << endl;
	
	dump(is, "wizard");
	
	if(version >= INNO_VERSION(2, 0, 0)) {
		dump(is, "wizard_small");
	}
	
	if(header.compression == stream::BZip2
	   || (header.compression == stream::LZMA1 && version == INNO_VERSION(4, 1, 5))
	   || (header.compression == stream::Zlib && version >= INNO_VERSION(4, 2, 6))) {
		
		dump(is, "decompressor");
	}
	
	if(is.fail()) {
		log_error << "error reading misc setup data";
	}
	
}

int main(int argc, char * argv[]) {
	
	color::init();
	
	logger::debug = true;
	
	if(argc <= 1) {
		std::cout << "usage: innoextract <Inno Setup installer>" << endl;
		return 1;
	}
	
	std::ifstream ifs(argv[1], std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	
	if(!ifs.is_open()) {
		log_error << "error opening file";
		return 1;
	}
	
	loader::offsets offsets;
	offsets.load(ifs);
	
	cout << std::boolalpha;
	
	cout << "loaded offsets:" << endl;
	if(offsets.exe_offset) {
		cout << "- exe: @ " << color::cyan << print_hex(offsets.exe_offset) << color::reset;
		if(offsets.exe_compressed_size) {
			cout << "  compressed: " << color::cyan << print_hex(offsets.exe_compressed_size)
			     << color::reset;
		}
		cout << "  uncompressed: " << color::cyan << print_bytes(offsets.exe_uncompressed_size)
		     << color::reset << endl;
		cout << "- exe checksum: " << color::cyan << offsets.exe_checksum  << color::reset << endl;
	}
	cout << if_not_zero("- message offset", print_hex(offsets.message_offset));
	cout << "- header offset: " << color::cyan << print_hex(offsets.header_offset)
	     << color::reset << endl;
	cout << if_not_zero("- data offset", print_hex(offsets.data_offset));
	
	ifs.seekg(offsets.header_offset);
	
	setup::version version;
	version.load(ifs);
	if(ifs.fail()) {
		log_error << "error reading setup data version!";
		return 1;
	}
	
	cout << "version: " << color::white << version << color::reset << endl;
	
	stream::block_reader::pointer is = stream::block_reader::get(ifs, version);
	if(!is) {
		log_error << "error reading block";
		return 1;
	}
	
	is->exceptions(std::ios_base::badbit | std::ios_base::failbit);
	
	setup::header header;
	header.load(*is, version);
	if(is->fail()) {
		log_error << "error reading setup data header!";
		return 1;
	}
	
	cout << endl;
	
	cout << if_not_empty("App name", header.app_name);
	cout << if_not_empty("App ver name", header.app_versioned_name);
	cout << if_not_empty("App id", header.app_id);
	cout << if_not_empty("Copyright", header.app_copyright);
	cout << if_not_empty("Publisher", header.app_publisher);
	cout << if_not_empty("Publisher URL", header.app_publisher_url);
	cout << if_not_empty("Support phone", header.app_support_phone);
	cout << if_not_empty("Support URL", header.app_support_url);
	cout << if_not_empty("Updates URL", header.app_updates_url);
	cout << if_not_empty("Version", header.app_version);
	cout << if_not_empty("Default dir name", header.default_dir_name);
	cout << if_not_empty("Default group name", header.default_group_name);
	cout << if_not_empty("Uninstall icon name", header.uninstall_icon_name);
	cout << if_not_empty("Base filename", header.base_filename);
	cout << if_not_empty("Uninstall files dir", header.uninstall_files_dir);
	cout << if_not_empty("Uninstall display name", header.uninstall_name);
	cout << if_not_empty("Uninstall display icon", header.uninstall_icon);
	cout << if_not_empty("App mutex", header.app_mutex);
	cout << if_not_empty("Default user name", header.default_user_name);
	cout << if_not_empty("Default user org", header.default_user_organisation);
	cout << if_not_empty("Default user serial", header.default_serial);
	cout << if_not_empty("Readme", header.app_readme_file);
	cout << if_not_empty("Contact", header.app_contact);
	cout << if_not_empty("Comments", header.app_comments);
	cout << if_not_empty("Modify path", header.app_modify_path);
	cout << if_not_empty("Uninstall reg key", header.create_uninstall_registry_key);
	cout << if_not_empty("Uninstallable", header.uninstallable);
	cout << if_not_empty("License", header.license_text);
	cout << if_not_empty("Info before text", header.info_before);
	cout << if_not_empty("Info after text", header.info_after);
	cout << if_not_empty("Uninstaller signature", header.uninstaller_signature);
	cout << if_not_empty("Compiled code", header.compiled_code);
	
	cout << if_not_zero("Lead bytes", header.lead_bytes);
	
	cout << if_not_zero("Language entries", header.language_count);
	cout << if_not_zero("Custom message entries", header.message_count);
	cout << if_not_zero("Permission entries", header.permission_count);
	cout << if_not_zero("Type entries", header.type_count);
	cout << if_not_zero("Component entries", header.component_count);
	cout << if_not_zero("Task entries", header.task_count);
	cout << if_not_zero("Dir entries", header.directory_count);
	cout << if_not_zero("File entries", header.file_count);
	cout << if_not_zero("File location entries", header.data_entry_count);
	cout << if_not_zero("Icon entries", header.icon_count);
	cout << if_not_zero("Ini entries", header.ini_entry_count);
	cout << if_not_zero("Registry entries", header.registry_entry_count);
	cout << if_not_zero("Delete entries", header.delete_entry_count);
	cout << if_not_zero("Uninstall delete entries", header.uninstall_delete_entry_count);
	cout << if_not_zero("Run entries", header.run_entry_count);
	cout << if_not_zero("Uninstall run entries", header.uninstall_run_entry_count);
	
	cout << if_not_equal("Min version", header.winver.begin, setup::windows_version::none);
	cout << if_not_equal("Only below version", header.winver.end, setup::windows_version::none);
	
	cout << std::hex;
	cout << if_not_zero("Back color", header.back_color);
	cout << if_not_zero("Back color2", header.back_color2);
	cout << if_not_zero("Wizard image back color", header.image_back_color);
	cout << if_not_zero("Wizard small image back color", header.small_image_back_color);
	cout << std::dec;
	
	if(header.options & (setup::header::Password | setup::header::EncryptionUsed)) {
		cout << "Password: " << color::cyan << header.password << color::reset << endl;
		// TODO print salt
	}
	
	cout << if_not_zero("Extra disk space required", header.extra_disk_space_required);
	cout << if_not_zero("Slices per disk", header.slices_per_disk);
	
	cout << if_not_equal("Install mode", header.install_mode, setup::header::NormalInstallMode);
	cout << "Uninstall log mode: " << color::cyan << header.uninstall_log_mode
	     << color::reset << endl;
	cout << "Uninstall style: " << color::cyan << header.uninstall_style << color::reset << endl;
	cout << "Dir exists warning: " << color::cyan << header.dir_exists_warning
	     << color::reset << endl;
	cout << if_not_equal("Privileges required", header.privileges_required, setup::header::NoPrivileges);
	cout << "Show language dialog: " << color::cyan << header.show_language_dialog
	     << color::reset << endl;
	cout << if_not_equal("Danguage detection", header.language_detection,
	              setup::header::NoLanguageDetection);
	cout << "Compression: " << color::cyan << header.compression << color::reset << endl;
	cout << "Architectures allowed: " << color::cyan << header.architectures_allowed
	     << color::reset << endl;
	cout << "Architectures installed in 64-bit mode: " << color::cyan
	     << header.architectures_installed_in_64bit_mode << color::reset << endl;
	
	if(header.options & setup::header::SignedUninstaller) {
		cout << if_not_zero("Size before signing uninstaller", header.signed_uninstaller_original_size);
		cout << if_not_zero("Uninstaller header checksum", header.signed_uninstaller_header_checksum);
	}
	
	cout << "Disable dir page: " << color::cyan << header.disable_dir_page << color::reset << endl;
	cout << "Disable program group page: " << color::cyan << header.disable_program_group_page
	     << color::reset << endl;
	
	cout << if_not_zero("Uninstall display size", header.uninstall_display_size);
	
	cout << "Options: " << color::green << header.options << color::reset << endl;
	
	cout << color::reset;
	
	if(header.language_count) {
		cout << endl << "Language entries:" << endl;
	}
	std::vector<setup::language_entry> languages;
	languages.resize(header.language_count);
	for(size_t i = 0; i < header.language_count; i++) {
		
		setup::language_entry & entry = languages[i];
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading language entry #" << i;
		}
		
		cout << " - " << quoted(entry.name) << ':' << endl;
		cout << if_not_empty("  Language name", entry.language_name);
		cout << if_not_empty("  Dialog font", entry.dialog_font);
		cout << if_not_empty("  Title font", entry.title_font);
		cout << if_not_empty("  Welcome font", entry.welcome_font);
		cout << if_not_empty("  Copyright font", entry.copyright_font);
		cout << if_not_empty("  Data", entry.data);
		cout << if_not_empty("  License", entry.license_text);
		cout << if_not_empty("  Info before text", entry.info_before);
		cout << if_not_empty("  Info after text", entry.info_after);
		
		cout << "  Language id: " << color::cyan << std::hex << entry.language_id << std::dec
		     << color::reset << endl;
		
		cout << if_not_zero("  Codepage", entry.codepage);
		cout << if_not_zero("  Dialog font size", entry.dialog_font_size);
		cout << if_not_zero("  Dialog font standard height", entry.dialog_font_standard_height);
		cout << if_not_zero("  Title font size", entry.title_font_size);
		cout << if_not_zero("  Welcome font size", entry.welcome_font_size);
		cout << if_not_zero("  Copyright font size", entry.copyright_font_size);
		cout << if_not_equal("  Right to left", entry.right_to_left, false);
		
	};
	
	if(version < INNO_VERSION(4, 0, 0)) {
		readWizardImageAndDecompressor(*is, version, header);
	}
	
	if(header.message_count) {
		cout << endl << "Message entries:" << endl;
	}
	for(size_t i = 0; i < header.message_count; i++) {
		
		setup::message_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading custom message entry #" << i;
		}
		
		if(entry.language >= 0 ? size_t(entry.language) >= languages.size() : entry.language != -1) {
			log_warning << "unexpected language index: " << entry.language;
		}
		
		uint32_t codepage;
		if(entry.language < 0) {
			codepage = version.codepage();
		} else {
			codepage = languages[size_t(entry.language)].codepage;
		}
		
		string decoded;
		to_utf8(entry.value, decoded, codepage);
		
		cout << " - " << quoted(entry.name);
		if(entry.language < 0) {
			cout << " (default) = ";
		} else {
			cout << " (" << color::cyan << languages[size_t(entry.language)].name
			     << color::reset << ") = ";
		}
		cout << quoted(decoded) << endl;
		
	}
	
	if(header.permission_count) {
		cout << endl << "Permission entries:" << endl;
	}
	for(size_t i = 0; i < header.permission_count; i++) {
		
		setup::permission_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading permission entry #" << i;
		}
		
		cout << " - " << entry.permissions.length() << " bytes";
		
	}
	
	if(header.type_count) {
		cout << endl << "Type entries:" << endl;
	}
	for(size_t i = 0; i < header.type_count; i++) {
		
		setup::type_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading type entry #" << i;
		}
		
		cout << " - " << quoted(entry.name) << ':' << endl;
		cout << if_not_empty("  Description", entry.description);
		cout << if_not_empty("  Languages", entry.languages);
		cout << if_not_empty("  Check", entry.check);
		
		print(cout, entry.winver, header);
		
		cout << if_not_equal("  Custom setup type", entry.custom_type, false);
		cout << if_not_equal("  Type", entry.type, setup::type_entry::User);
		cout << if_not_zero("  Size", entry.size);
		
	}
	
	if(header.component_count) {
		cout << endl << "Component entries:" << endl;
	}
	for(size_t i = 0; i < header.component_count; i++) {
		
		setup::component_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading component entry #" << i;
		}
		
		cout << " - " << quoted(entry.name) << ':' << endl;
		cout << if_not_empty("  Types", entry.types);
		cout << if_not_empty("  Description", entry.description);
		cout << if_not_empty("  Languages", entry.languages);
		cout << if_not_empty("  Check", entry.check);
		
		cout << if_not_zero("  Extra disk space required", entry.extra_disk_pace_required);
		cout << if_not_zero("  Level", entry.level);
		cout << if_not_equal("  Used", entry.used, true);
		
		print(cout, entry.winver, header);
		
		cout << if_not_zero("  Options", entry.options);
		cout << if_not_zero("  Size", entry.size);
		
	}
	
	if(header.task_count) {
		cout << endl << "Task entries:" << endl;
	}
	for(size_t i = 0; i < header.task_count; i++) {
		
		setup::task_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading task entry #" << i;
		}
		
		cout << " - " << quoted(entry.name) << ':' << endl;
		cout << if_not_empty("  Description", entry.description);
		cout << if_not_empty("  Group description", entry.group_description);
		cout << if_not_empty("  Components", entry.components);
		cout << if_not_empty("  Languages", entry.languages);
		cout << if_not_empty("  Check", entry.check);
		
		cout << if_not_zero("  Level", entry.level);
		cout << if_not_equal("  Used", entry.used, true);
		
		print(cout, entry.winver, header);
		
		cout << if_not_zero("  Options", entry.options);
		
	}
	
	if(header.directory_count) {
		cout << endl << "Directory entries:" << endl;
	}
	for(size_t i = 0; i < header.directory_count; i++) {
		
		setup::directory_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading directory entry #" << i;
		}
		
		cout << " - " << quoted(entry.name) << ':' << endl;
		
		print(cout, entry, header);
		
		if(!entry.permissions.empty()) {
			cout << "  Permissions: " << entry.permissions.length() << " bytes";
		}
		
		
		cout << if_not_zero("  Attributes", entry.attributes);
		
		cout << if_not_equal("  Permission entry", entry.permission, -1);
		
		cout << if_not_zero("  Options", entry.options);
		
	}
	
	if(header.file_count) {
		cout << endl << "File entries:" << endl;
	}
	std::vector<setup::file_entry> files;
	files.resize(header.file_count);
	for(size_t i = 0; i < header.file_count; i++) {
		
		setup::file_entry & entry = files[i];
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading file entry #" << i;
		}
		
		if(entry.destination.empty()) {
			cout << " - File #" << i;
		} else {
			cout << " - " << quoted(entry.destination);
		}
		if(entry.location != uint32_t(-1)) {
			cout << " (location: " << color::cyan << entry.location << color::reset << ')';
		}
		cout  << endl;
		
		cout << if_not_empty("  Source", entry.source);
		cout << if_not_empty("  Install font name", entry.install_font_name);
		cout << if_not_empty("  Strong assembly name", entry.strong_assembly_name);
		
		print(cout, entry, header);
		
		cout << if_not_zero("  Attributes", entry.attributes);
		cout << if_not_zero("  Size", entry.external_size);
		
		cout << if_not_equal("  Permission entry", entry.permission, -1);
		
		cout << if_not_zero("  Options", entry.options);
		
		cout << if_not_equal("  Type", entry.type, setup::file_entry::UserFile);
		
	}
	
	if(header.icon_count) {
		cout << endl << "Icon entries:" << endl;
	}
	for(size_t i = 0; i < header.icon_count; i++) {
		
		setup::icon_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading icon entry #" << i;
		}
		
		cout << " - " << quoted(entry.name) << " -> " << quoted(entry.filename) << endl;
		cout << if_not_empty("  Parameters", entry.parameters);
		cout << if_not_empty("  Working directory", entry.working_dir);
		cout << if_not_empty("  Icon file", entry.icon_file);
		cout << if_not_empty("  Comment", entry.comment);
		cout << if_not_empty("  App user model id", entry.app_user_model_id);
		
		print(cout, entry, header);
		
		cout << if_not_zero("  Icon index", entry.icon_index);
		cout << if_not_equal("  Show command", entry.show_command, 1);
		cout << if_not_equal("  Close on exit", entry.close_on_exit, setup::icon_entry::NoSetting);
		
		cout << if_not_zero("  Hotkey", entry.hotkey);
		
		cout << if_not_zero("  Options", entry.options);
		
	}
	
	if(header.ini_entry_count) {
		cout << endl << "Ini entries:" << endl;
	}
	for(size_t i = 0; i < header.ini_entry_count; i++) {
		
		setup::ini_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading ini entry #" << i;
		}
		
		cout << " - in " << quoted(entry.inifile);
		cout << " set [" << quoted(entry.section) << "] ";
		cout << quoted(entry.key) << " = " << quoted(entry.value) << std::endl;
		
		print(cout, entry, header);
		
		cout << if_not_zero("  Options", entry.options);
		
	}
	
	if(header.registry_entry_count) {
		cout << endl << "Registry entries:" << endl;
	}
	for(size_t i = 0; i < header.registry_entry_count; i++) {
		
		setup::registry_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading registry entry #" << i;
		}
		
		cout << " - ";
		if(entry.hive != setup::registry_entry::Unset) {
			cout << entry.hive << '\\';
		}
		cout << quoted(entry.key);
		cout << endl << "  ";
		if(entry.name.empty()) {
			cout << "(default)";
		} else {
			cout << quoted(entry.name);
		}
		if(!entry.value.empty()) {
			cout << " = " << quoted(entry.value);
		}
		if(entry.type != setup::registry_entry::None) {
			cout << " (" << color::cyan << entry.type << color::reset << ')';
		}
		cout << endl;
		
		print(cout, entry, header);
		
		if(!entry.permissions.empty()) {
			cout << "  Permissions: " << entry.permissions.length() << " bytes";
		}
		cout << if_not_equal("  Permission entry", entry.permission, -1);
		
		cout << if_not_zero("  Options", entry.options);
		
	}
	
	if(header.delete_entry_count) {
		cout << endl << "Delete entries:" << endl;
	}
	for(size_t i = 0; i < header.delete_entry_count; i++) {
		
		setup::delete_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading install delete entry #" << i;
		}
		
		cout << " - " << quoted(entry.name)
		     << " (" << color::cyan << entry.type << color::reset << ')' << endl;
		
		print(cout, entry, header);
		
	}
	
	if(header.uninstall_delete_entry_count) {
		cout << endl << "Uninstall delete entries:" << endl;
	}
	for(size_t i = 0; i < header.uninstall_delete_entry_count; i++) {
		
		setup::delete_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading uninstall delete entry #" << i;
		}
		
		cout << " - " << quoted(entry.name)
		     << " (" << color::cyan << entry.type << color::reset << ')' << endl;
		
		print(cout, entry, header);
		
	}
	
	if(header.run_entry_count) {
		cout << endl << "Run entries:" << endl;
	}
	for(size_t i = 0; i < header.run_entry_count; i++) {
		
		setup::run_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading install run entry #" << i;
		}
		
		print(cout, entry, header);
		
	}
	
	if(header.uninstall_run_entry_count) {
		cout << endl << "Uninstall run entries:" << endl;
	}
	for(size_t i = 0; i < header.uninstall_run_entry_count; i++) {
		
		setup::run_entry entry;
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading uninstall run entry #" << i;
		}
		
		print(cout, entry, header);
		
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		readWizardImageAndDecompressor(*is, version, header);
	}
	
	{
		is->exceptions(std::ios_base::goodbit);
		char dummy;
		if(!is->get(dummy).eof()) {
			log_warning << "expected end of stream";
		}
	}
	
	// TODO skip to end if not there yet
	
	is = stream::block_reader::get(ifs, version);
	if(!is) {
		log_error << "error reading block";
		return 1;
	}
	
	is->exceptions(std::ios_base::badbit | std::ios_base::failbit);
	
	if(header.data_entry_count) {
		cout << endl << "File location entries:" << endl;
	}
	std::vector<setup::data_entry> locations;
	locations.resize(header.data_entry_count);
	for(size_t i = 0; i < header.data_entry_count; i++) {
		
		setup::data_entry & entry = locations[i];
		entry.load(*is, version);
		if(is->fail()) {
			log_error << "error reading file location entry #" << i;
		}
		
		cout << " - " << "File location #" << i << ':' << endl;
		
		cout << if_not_zero("  First slice", entry.chunk.first_slice);
		cout << if_not_equal("  Last slice", entry.chunk.last_slice, entry.chunk.first_slice);
		
		cout << "  Chunk: offset " << color::cyan << print_hex(entry.chunk.offset)
		     << color::reset  << " size " << color::cyan << print_hex(entry.chunk.size)
				 << color::reset << std::endl;
		
		cout << if_not_zero("  File offset", print_hex(entry.file.offset));
		cout << if_not_zero("  File size", print_bytes(entry.file.size));
		
		cout << "  Checksum: " << entry.file.checksum << endl;
		
		std::tm t;
		if(entry.options & setup::data_entry::TimeStampInUTC) {
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
		
		cout << if_not_zero("  Options", entry.options);
		
		if(entry.options & setup::data_entry::VersionInfoValid) {
			cout << if_not_zero("  File version LS", entry.file_version_ls);
			cout << if_not_zero("  File version MS", entry.file_version_ms);
		}
		
	}
	
	{
		is->exceptions(std::ios_base::goodbit);
		char dummy;
		if(!is->get(dummy).eof()) {
			log_warning << "expected end of stream";
		}
	}
	
	is.reset();
	
	std::vector<std::vector<size_t> > files_for_location;
	files_for_location.resize(locations.size());
	for(size_t i = 0; i < files.size(); i++) {
		if(files[i].location < files_for_location.size()) {
			files_for_location[files[i].location].push_back(i);
		}
	}
	
	typedef std::map<stream::file, size_t> Files;
	typedef std::map<stream::chunk, Files> Chunks;
	Chunks chunks;
	for(size_t i = 0; i < locations.size(); i++) {
		setup::data_entry & location = locations[i];
		if(location.chunk.compression == stream::UnknownCompression) {
			location.chunk.compression = header.compression;
		}
		chunks[location.chunk][location.file] = i;
	}
	
	boost::shared_ptr<stream::slice_reader> slice_reader;
	
	if(offsets.data_offset) {
		slice_reader = boost::make_shared<stream::slice_reader>(argv[1], offsets.data_offset);
	} else {
		fs::path path(argv[1]);
		slice_reader = boost::make_shared<stream::slice_reader>(path.parent_path().string() + '/',
		                                                        path.stem().string(),
		                                                        header.slices_per_disk);
	}
	
	try {
	
	BOOST_FOREACH(const Chunks::value_type & chunk, chunks) {
		
		cout << "[starting " << chunk.first.compression << " chunk @ " << chunk.first.first_slice
		     << " + " << print_hex(offsets.data_offset) << " + " << print_hex(chunk.first.offset)
		     << ']' << std::endl;
		
		stream::chunk_reader::pointer chunk_source;
		chunk_source = stream::chunk_reader::get(*slice_reader, chunk.first);
		
		uint64_t offset = 0;
		
		BOOST_FOREACH(const Files::value_type & location, chunk.second) {
			const stream::file & file = location.first;
			
			if(file.offset < offset) {
				log_error << "bad offset";
				return 1;
			}
			
			if(file.offset > offset) {
				std::cout << "discarding " << print_bytes(file.offset - offset) << std::endl;
				discard(*chunk_source, file.offset - offset);
			}
			offset = file.offset + file.size;
			
			std::cout << "-> reading ";
			bool named = false;
			BOOST_FOREACH(size_t file_i, files_for_location[location.second]) {
				if(!files[file_i].destination.empty()) {
					std::cout << '"' << files[file_i].destination << '"';
					named = true;
					break;
				}
			}
			if(!named) {
				std::cout << "unnamed file";
			}
			std::cout << " @ " << print_hex(file.offset)
			          << " (" << print_bytes(file.size) << ')' << std::endl;
			
			crypto::checksum checksum;
			
			stream::file_reader::pointer file_source;
			file_source = stream::file_reader::get(*chunk_source, file, &checksum);
			
			BOOST_FOREACH(size_t file_i, files_for_location[location.second]) {
				if(!files[file_i].destination.empty()) {
					std::ofstream ofs(files[file_i].destination.c_str());
					
					char buffer[8192 * 10];
					
					float status = 0.f;
					uint64_t total = 0;
					
					std::ostringstream oss;
					float last_rate = 0;
					
					int64_t last_milliseconds = 0;
					
					boost::posix_time::ptime start(boost::posix_time::microsec_clock::universal_time());
					
					while(!file_source->eof()) {
						
						std::streamsize n = file_source->read(buffer, ARRAY_SIZE(buffer)).gcount();
						
						if(n > 0) {
							
							ofs.write(buffer, n);
							
							total += uint64_t(n);
							float new_status = float(size_t(1000.f * float(total) / float(file.size)))
							                   * (1 / 1000.f);
							if(status != new_status && new_status != 100.f) {
								
								boost::posix_time::ptime now(boost::posix_time::microsec_clock::universal_time());
								int64_t milliseconds = (now - start).total_milliseconds();
								
								if(milliseconds - last_milliseconds > 200) {
									last_milliseconds = milliseconds;
									
									if(total >= 10 * 1024 && milliseconds > 0) {
										float rate = 1000.f * float(total) / float(milliseconds);
										if(rate != last_rate) {
											last_rate = rate;
											oss.str(string()); // clear the buffer
											oss << std::right << std::fixed << std::setfill(' ') << std::setw(8)
											    << print_bytes(rate) << "/s";
										}
									}
									
									status = new_status;
									progress::show(status, oss.str());
								}
							}
						}
					}
					
					break; // TODO ...
				}
			}
			
			progress::clear();
			
			if(checksum != file.checksum) {
				log_warning << "checksum mismatch:";
				log_warning << "actual:   " << checksum;
				log_warning << "expected: " << file.checksum;
			}
		}
	}
	
	} catch(std::ios_base::failure e) {
		log_error << e.what();
	}
	
	std::cout << color::green << "Done" << color::reset << std::dec;
	
	if(logger::total_errors || logger::total_warnings) {
		std::cout << " with ";
		if(logger::total_errors) {
			std::cout << color::red << logger::total_errors << " errors" << color::reset;
		}
		if(logger::total_errors && logger::total_warnings) {
			std::cout << " and ";
		}
		if(logger::total_warnings) {
			std::cout << color::yellow << logger::total_warnings << " warnings" << color::reset;
		}
	}
	
	std::cout << '.' << std::endl;
	
	return 0;
}

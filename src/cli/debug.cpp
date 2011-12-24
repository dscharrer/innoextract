
#include "cli/debug.hpp"

#include <ctime>

#include <boost/foreach.hpp>

#include "loader/offsets.hpp"
#include "setup/component.hpp"
#include "setup/data.hpp"
#include "setup/delete.hpp"
#include "setup/directory.hpp"
#include "setup/file.hpp"
#include "setup/header.hpp"
#include "setup/icon.hpp"
#include "setup/info.hpp"
#include "setup/ini.hpp"
#include "setup/item.hpp"
#include "setup/language.hpp"
#include "setup/message.hpp"
#include "setup/permission.hpp"
#include "setup/registry.hpp"
#include "setup/run.hpp"
#include "setup/task.hpp"
#include "setup/type.hpp"
#include "setup/version.hpp"
#include "stream/block.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/output.hpp"
#include "util/util.hpp"

using std::cout;

void print_offsets(const loader::offsets & offsets) {
	
	cout << "loaded offsets:" << '\n';
	if(offsets.exe_offset) {
		cout << "- exe: @ " << color::cyan << print_hex(offsets.exe_offset) << color::reset;
		if(offsets.exe_compressed_size) {
			cout << "  compressed: " << color::cyan << print_hex(offsets.exe_compressed_size)
			     << color::reset;
		}
		cout << "  uncompressed: " << color::cyan << print_bytes(offsets.exe_uncompressed_size)
		     << color::reset << '\n';
		cout << "- exe checksum: " << color::cyan << offsets.exe_checksum  << color::reset << '\n';
	}
	cout << if_not_zero("- message offset", print_hex(offsets.message_offset));
	cout << "- header offset: " << color::cyan << print_hex(offsets.header_offset)
	     << color::reset << '\n';
	cout << if_not_zero("- data offset", print_hex(offsets.data_offset));
}

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

static void print_entry(const setup::info & info, size_t i, const setup::language_entry & entry) {
	
	(void)info, (void)i;
	
	cout << " - " << quoted(entry.name) << ':' << '\n';
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
	     << color::reset << '\n';
	
	cout << if_not_zero("  Codepage", entry.codepage);
	cout << if_not_zero("  Dialog font size", entry.dialog_font_size);
	cout << if_not_zero("  Dialog font standard height", entry.dialog_font_standard_height);
	cout << if_not_zero("  Title font size", entry.title_font_size);
	cout << if_not_zero("  Welcome font size", entry.welcome_font_size);
	cout << if_not_zero("  Copyright font size", entry.copyright_font_size);
	cout << if_not_equal("  Right to left", entry.right_to_left, false);
}

static void print_entry(const setup::info & info, size_t i, const setup::message_entry & entry) {
	
	(void)i;
	
	cout << " - " << quoted(entry.name);
	if(entry.language < 0) {
		cout << " (default) = ";
	} else {
		cout << " (" << color::cyan << info.languages[size_t(entry.language)].name
		     << color::reset << ") = ";
	}
	cout << quoted(entry.value) << '\n';
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::permission_entry & entry) {
	
	(void)info, (void)i;
	
	cout << " - " << print_bytes(entry.permissions.length()) << '\n';
}

static void print_entry(const setup::info & info, size_t i, const setup::type_entry & entry) {
	
	(void)i;
	
	cout << " - " << quoted(entry.name) << ':' << '\n';
	cout << if_not_empty("  Description", entry.description);
	cout << if_not_empty("  Languages", entry.languages);
	cout << if_not_empty("  Check", entry.check);
	
	print(cout, entry.winver, info.header);
	
	cout << if_not_equal("  Custom setup type", entry.custom_type, false);
	cout << if_not_equal("  Type", entry.type, setup::type_entry::User);
	cout << if_not_zero("  Size", entry.size);
}

static void print_entry(const setup::info & info, size_t i, const setup::component_entry & entry) {
	
	(void)i;
	
	cout << " - " << quoted(entry.name) << ':' << '\n';
	cout << if_not_empty("  Types", entry.types);
	cout << if_not_empty("  Description", entry.description);
	cout << if_not_empty("  Languages", entry.languages);
	cout << if_not_empty("  Check", entry.check);
	cout << if_not_zero("  Extra disk space required", entry.extra_disk_pace_required);
	cout << if_not_zero("  Level", entry.level);
	cout << if_not_equal("  Used", entry.used, true);
	
	print(cout, entry.winver, info.header);
	
	cout << if_not_zero("  Options", entry.options);
	cout << if_not_zero("  Size", entry.size);
}

static void print_entry(const setup::info & info, size_t i, const setup::task_entry & entry) {
	
	(void)i;
	
	cout << " - " << quoted(entry.name) << ':' << '\n';
	cout << if_not_empty("  Description", entry.description);
	cout << if_not_empty("  Group description", entry.group_description);
	cout << if_not_empty("  Components", entry.components);
	cout << if_not_empty("  Languages", entry.languages);
	cout << if_not_empty("  Check", entry.check);
	cout << if_not_zero("  Level", entry.level);
	cout << if_not_equal("  Used", entry.used, true);
	
	print(cout, entry.winver, info.header);
	
	cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i, const setup::directory_entry & entry) {
	
	(void)i;
	
	cout << " - " << quoted(entry.name) << ':' << '\n';
	
	print(cout, entry, info.header);
	
	if(!entry.permissions.empty()) {
		cout << "  Permissions: " << entry.permissions.length() << " bytes";
	}
	
	cout << if_not_zero("  Attributes", entry.attributes);
	cout << if_not_equal("  Permission entry", entry.permission, -1);
	cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i, const setup::file_entry & entry) {
	
	if(entry.destination.empty()) {
		cout << " - File #" << i;
	} else {
		cout << " - " << quoted(entry.destination);
	}
	if(entry.location != uint32_t(-1)) {
		cout << " (location: " << color::cyan << entry.location << color::reset << ')';
	}
	cout  << '\n';
	
	cout << if_not_empty("  Source", entry.source);
	cout << if_not_empty("  Install font name", entry.install_font_name);
	cout << if_not_empty("  Strong assembly name", entry.strong_assembly_name);
	
	print(cout, entry, info.header);
	
	cout << if_not_zero("  Attributes", entry.attributes);
	cout << if_not_zero("  Size", entry.external_size);
	cout << if_not_equal("  Permission entry", entry.permission, -1);
	cout << if_not_zero("  Options", entry.options);
	cout << if_not_equal("  Type", entry.type, setup::file_entry::UserFile);
}

static void print_entry(const setup::info & info, size_t i, const setup::icon_entry & entry) {
	
	(void)i;
	
	cout << " - " << quoted(entry.name) << " -> " << quoted(entry.filename) << '\n';
	cout << if_not_empty("  Parameters", entry.parameters);
	cout << if_not_empty("  Working directory", entry.working_dir);
	cout << if_not_empty("  Icon file", entry.icon_file);
	cout << if_not_empty("  Comment", entry.comment);
	cout << if_not_empty("  App user model id", entry.app_user_model_id);
	
	print(cout, entry, info.header);
	
	cout << if_not_zero("  Icon index", entry.icon_index);
	cout << if_not_equal("  Show command", entry.show_command, 1);
	cout << if_not_equal("  Close on exit", entry.close_on_exit, setup::icon_entry::NoSetting);
	cout << if_not_zero("  Hotkey", entry.hotkey);
	cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i, const setup::ini_entry & entry) {
	
	(void)i;
	
	cout << " - in " << quoted(entry.inifile);
	cout << " set [" << quoted(entry.section) << "] ";
	cout << quoted(entry.key) << " = " << quoted(entry.value) << '\n';
	
	print(cout, entry, info.header);
	
	cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i, const setup::registry_entry & entry) {
	
	(void)i;
	
	cout << " - ";
	if(entry.hive != setup::registry_entry::Unset) {
		cout << entry.hive << '\\';
	}
	cout << quoted(entry.key);
	cout << '\n' << "  ";
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
	cout << '\n';
	
	print(cout, entry, info.header);
	
	if(!entry.permissions.empty()) {
		cout << "  Permissions: " << entry.permissions.length() << " bytes";
	}
	cout << if_not_equal("  Permission entry", entry.permission, -1);
	cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i, const setup::delete_entry & entry) {
	
	(void)i;
	
	cout << " - " << quoted(entry.name)
	     << " (" << color::cyan << entry.type << color::reset << ')' << '\n';
	
	print(cout, entry, info.header);
}

static void print_entry(const setup::info & info, size_t i, const setup::run_entry & entry) {
	
	(void)i;
	
	cout << " - " << quoted(entry.name) << ':' << '\n';
	cout << if_not_empty("  Parameters", entry.parameters);
	cout << if_not_empty("  Working directory", entry.working_dir);
	cout << if_not_empty("  Run once id", entry.run_once_id);
	cout << if_not_empty("  Status message", entry.status_message);
	cout << if_not_empty("  Verb", entry.verb);
	cout << if_not_empty("  Description", entry.verb);
	
	print(cout, entry, info.header);
	
	cout << if_not_equal("  Show command", entry.show_command, 1);
	cout << if_not_equal("  Wait", entry.wait, setup::run_entry::WaitUntilTerminated);
	cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i, const setup::data_entry & entry) {
	
	(void)info;
	
	cout << " - " << "File location #" << i << ':' << '\n';
	cout << if_not_zero("  First slice", entry.chunk.first_slice);
	cout << if_not_equal("  Last slice", entry.chunk.last_slice, entry.chunk.first_slice);
	cout << "  Chunk: offset " << color::cyan << print_hex(entry.chunk.offset)
	     << color::reset << " size " << color::cyan << print_hex(entry.chunk.size)
	     << color::reset << '\n';
	cout << if_not_zero("  File offset", print_hex(entry.file.offset));
	cout << if_not_zero("  File size", print_bytes(entry.file.size));
	
	cout << "  Checksum: " << entry.file.checksum << '\n';
	
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
	     << color::reset << " +" << entry.timestamp.tv_nsec << '\n';
	
	cout << if_not_zero("  Options", entry.options);
	
	if(entry.options & setup::data_entry::VersionInfoValid) {
		cout << if_not_zero("  File version LS", entry.file_version_ls);
		cout << if_not_zero("  File version MS", entry.file_version_ms);
	}
}

template <class Entry>
static void print_entries(const setup::info & info, const std::vector<Entry> & entries,
                          const std::string & name) {
	
	if(entries.empty()) {
		return;
	}
	
	cout << '\n' << name << ":\n";
	for(size_t i = 0; i < entries.size(); i++) {
		print_entry(info, i, entries[i]);
	}
}

static void print_header(const setup::header & header) {
	
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
		cout << "Password: " << color::cyan << header.password << color::reset << '\n';
		// TODO print salt
	}
	
	cout << if_not_zero("Extra disk space required", header.extra_disk_space_required);
	cout << if_not_zero("Slices per disk", header.slices_per_disk);
	
	cout << if_not_equal("Install mode", header.install_mode, setup::header::NormalInstallMode);
	cout << "Uninstall log mode: " << color::cyan << header.uninstall_log_mode
	     << color::reset << '\n';
	cout << "Uninstall style: " << color::cyan << header.uninstall_style << color::reset << '\n';
	cout << "Dir exists warning: " << color::cyan << header.dir_exists_warning
	     << color::reset << '\n';
	cout << if_not_equal("Privileges required", header.privileges_required,
	                                            setup::header::NoPrivileges);
	cout << "Show language dialog: " << color::cyan << header.show_language_dialog
	     << color::reset << '\n';
	cout << if_not_equal("Danguage detection", header.language_detection,
	              setup::header::NoLanguageDetection);
	cout << "Compression: " << color::cyan << header.compression << color::reset << '\n';
	cout << "Architectures allowed: " << color::cyan << header.architectures_allowed
	     << color::reset << '\n';
	cout << "Architectures installed in 64-bit mode: " << color::cyan
	     << header.architectures_installed_in_64bit_mode << color::reset << '\n';
	
	if(header.options & setup::header::SignedUninstaller) {
		cout << if_not_zero("Size before signing uninstaller", header.signed_uninstaller_original_size);
		cout << if_not_zero("Uninstaller header checksum", header.signed_uninstaller_header_checksum);
	}
	
	cout << "Disable dir page: " << color::cyan << header.disable_dir_page << color::reset << '\n';
	cout << "Disable program group page: " << color::cyan << header.disable_program_group_page
	     << color::reset << '\n';
	
	cout << if_not_zero("Uninstall display size", header.uninstall_display_size);
	
	cout << "Options: " << color::green << header.options << color::reset << '\n';
	
	cout << color::reset;
}

static const char * magic_numbers[][2] = {
	{ "GIF89a", "gif" },
	{ "GIF87a", "gif" },
	{ "\xFF\xD8", "jpg" },
	{ "\x89PNG\r\n\x1A\n", "png" },
	{ "%PDF", "pdf" },
	{ "MZ", "dll" },
	{ "BM", "bmp" },
};

static const char * guess_extension(const std::string & data) {
	
	for(size_t i = 0; i < ARRAY_SIZE(magic_numbers); i++) {
		
		size_t n = strlen(magic_numbers[i][0]);
		
		if(!data.compare(0, n, magic_numbers[i][0], n)) {
			return magic_numbers[i][1];
		}
	}
	
	return "bin";
}

static void print_aux(const setup::info & info) {
	
	if(info.wizard_image.empty() && info.wizard_image_small.empty()
	   && info.decompressor_dll.empty()) {
		return;
	}
	
	cout << '\n';
	
	if(!info.wizard_image.empty()) {
		cout << "Wizard image: " << print_bytes(info.wizard_image.length()) << " ("
		     << guess_extension(info.wizard_image) << ")\n";
	}
	
	if(!info.wizard_image_small.empty()) {
		cout << "Wizard small image: " << print_bytes(info.wizard_image_small.length()) << " ("
		     << guess_extension(info.wizard_image_small) << ")\n";
	}
	
	if(!info.decompressor_dll.empty()) {
		cout << "Decompressor dll: " << print_bytes(info.decompressor_dll.length()) << " ("
		     << guess_extension(info.decompressor_dll) << ")\n";
	}
	
}

void print_info(const setup::info & info) {
	
	print_header(info.header);
	
	print_entries(info, info.languages, "Languages");
	print_entries(info, info.messages, "Messages");
	print_entries(info, info.permissions, "Permissions");
	print_entries(info, info.types, "Types");
	print_entries(info, info.components, "Components");
	print_entries(info, info.tasks, "Tasks");
	print_entries(info, info.directories, "Directories");
	print_entries(info, info.files, "Files");
	print_entries(info, info.icons, "Icons");
	print_entries(info, info.ini_entries, "INI entries");
	print_entries(info, info.registry_entries, "Registry entries");
	print_entries(info, info.delete_entries, "Delete entries");
	print_entries(info, info.uninstall_delete_entries, "Uninstall delete entries");
	print_entries(info, info.run_entries, "Run entries");
	print_entries(info, info.uninstall_run_entries, "Uninstall run entries");
	print_entries(info, info.data_entries, "Data entries");
	
	print_aux(info);
}

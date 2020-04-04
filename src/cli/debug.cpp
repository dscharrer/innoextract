/*
 * Copyright (C) 2011-2020 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "cli/debug.hpp"

#include <ctime>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/range/size.hpp>

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

#include "util/fstream.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/output.hpp"
#include "util/time.hpp"

namespace fs = boost::filesystem;

void print_offsets(const loader::offsets & offsets) {
	
	std::cout << "loaded offsets:" << '\n';
	if(offsets.exe_offset) {
		std::cout << "- exe: @ " << color::cyan << print_hex(offsets.exe_offset)
		          << color::reset;
		if(offsets.exe_compressed_size) {
			std::cout << "  compressed: " << color::cyan
			          << print_hex(offsets.exe_compressed_size) << color::reset;
		}
		std::cout << "  uncompressed: " << color::cyan
		          << print_bytes(offsets.exe_uncompressed_size) << color::reset;
		std::cout << "  checksum: " << color::cyan << offsets.exe_checksum
		          << color::reset << '\n';
	}
	std::cout << if_not_zero("- message offset", print_hex(offsets.message_offset));
	std::cout << "- header offset: " << color::cyan << print_hex(offsets.header_offset)
	          << color::reset << '\n';
	std::cout << if_not_zero("- data offset", print_hex(offsets.data_offset));
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

static void print_entry(const setup::info & info, size_t i,
                        const setup::language_entry & entry) {
	
	(void)info, (void)i;
	
	std::cout << " - " << quoted(entry.name) << ':' << '\n';
	std::cout << if_not_empty("  Language name", entry.language_name);
	std::cout << if_not_empty("  Dialog font", entry.dialog_font);
	std::cout << if_not_empty("  Title font", entry.title_font);
	std::cout << if_not_empty("  Welcome font", entry.welcome_font);
	std::cout << if_not_empty("  Copyright font", entry.copyright_font);
	std::cout << if_not_empty("  Data", entry.data);
	std::cout << if_not_empty("  License", entry.license_text);
	std::cout << if_not_empty("  Info before text", entry.info_before);
	std::cout << if_not_empty("  Info after text", entry.info_after);
	
	std::cout << "  Language id: " << color::cyan << std::hex << entry.language_id
	          << std::dec << color::reset << '\n';
	
	std::cout << if_not_zero("  Codepage", entry.codepage);
	std::cout << if_not_zero("  Dialog font size", entry.dialog_font_size);
	std::cout << if_not_zero("  Dialog font standard height",
	                         entry.dialog_font_standard_height);
	std::cout << if_not_zero("  Title font size", entry.title_font_size);
	std::cout << if_not_zero("  Welcome font size", entry.welcome_font_size);
	std::cout << if_not_zero("  Copyright font size", entry.copyright_font_size);
	std::cout << if_not_equal("  Right to left", entry.right_to_left, false);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::message_entry & entry) {
	
	(void)i;
	
	std::cout << " - " << quoted(entry.name);
	if(entry.language < 0) {
		std::cout << " (default) = ";
	} else {
		std::cout << " (" << color::cyan << info.languages[size_t(entry.language)].name
		          << color::reset << ") = ";
	}
	std::cout << quoted(entry.value) << '\n';
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::permission_entry & entry) {
	
	(void)info, (void)i;
	
	std::cout << " - " << print_bytes(entry.permissions.length()) << '\n';
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::type_entry & entry) {
	
	(void)i;
	
	std::cout << " - " << quoted(entry.name) << ':' << '\n';
	std::cout << if_not_empty("  Description", entry.description);
	std::cout << if_not_empty("  Languages", entry.languages);
	std::cout << if_not_empty("  Check", entry.check);
	
	print(std::cout, entry.winver, info.header);
	
	std::cout << if_not_equal("  Custom setup type", entry.custom_type, false);
	std::cout << if_not_equal("  Type", entry.type, setup::type_entry::User);
	std::cout << if_not_zero("  Size", entry.size);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::component_entry & entry) {
	
	(void)i;
	
	std::cout << " - " << quoted(entry.name) << ':' << '\n';
	std::cout << if_not_empty("  Types", entry.types);
	std::cout << if_not_empty("  Description", entry.description);
	std::cout << if_not_empty("  Languages", entry.languages);
	std::cout << if_not_empty("  Check", entry.check);
	std::cout << if_not_zero("  Extra disk space required", entry.extra_disk_pace_required);
	std::cout << if_not_zero("  Level", entry.level);
	std::cout << if_not_equal("  Used", entry.used, true);
	
	print(std::cout, entry.winver, info.header);
	
	std::cout << if_not_zero("  Options", entry.options);
	std::cout << if_not_zero("  Size", entry.size);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::task_entry & entry) {
	
	(void)i;
	
	std::cout << " - " << quoted(entry.name) << ':' << '\n';
	std::cout << if_not_empty("  Description", entry.description);
	std::cout << if_not_empty("  Group description", entry.group_description);
	std::cout << if_not_empty("  Components", entry.components);
	std::cout << if_not_empty("  Languages", entry.languages);
	std::cout << if_not_empty("  Check", entry.check);
	std::cout << if_not_zero("  Level", entry.level);
	std::cout << if_not_equal("  Used", entry.used, true);
	
	print(std::cout, entry.winver, info.header);
	
	std::cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::directory_entry & entry) {
	
	(void)i;
	
	std::cout << " - " << quoted(entry.name) << ':' << '\n';
	
	print(std::cout, entry, info.header);
	
	if(!entry.permissions.empty()) {
		std::cout << "  Permissions: " << entry.permissions.length() << " bytes";
	}
	
	std::cout << if_not_zero("  Attributes", entry.attributes);
	std::cout << if_not_equal("  Permission entry", entry.permission, boost::int16_t(-1));
	std::cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::file_entry & entry) {
	
	if(entry.destination.empty()) {
		std::cout << " - File #" << i;
	} else {
		std::cout << " - " << quoted(entry.destination);
	}
	if(entry.location != boost::uint32_t(-1)) {
		std::cout << " (location: " << color::cyan << entry.location << color::reset << ')';
	}
	std::cout  << '\n';
	
	std::cout << if_not_empty("  Source", entry.source);
	std::cout << if_not_empty("  Install font name", entry.install_font_name);
	std::cout << if_not_empty("  Strong assembly name", entry.strong_assembly_name);
	
	print(std::cout, entry, info.header);
	
	std::cout << if_not_zero("  Attributes", entry.attributes);
	std::cout << if_not_zero("  Size", entry.external_size);
	std::cout << if_not_equal("  Permission entry", entry.permission, boost::int16_t(-1));
	std::cout << if_not_zero("  Options", entry.options);
	std::cout << if_not_equal("  Type", entry.type, setup::file_entry::UserFile);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::icon_entry & entry) {
	
	(void)i;
	
	std::cout << " - " << quoted(entry.name) << " -> " << quoted(entry.filename) << '\n';
	std::cout << if_not_empty("  Parameters", entry.parameters);
	std::cout << if_not_empty("  Working directory", entry.working_dir);
	std::cout << if_not_empty("  Icon file", entry.icon_file);
	std::cout << if_not_empty("  Comment", entry.comment);
	std::cout << if_not_empty("  App user model id", entry.app_user_model_id);
	
	print(std::cout, entry, info.header);
	
	std::cout << if_not_zero("  Icon index", entry.icon_index);
	std::cout << if_not_equal("  Show command", entry.show_command, 1);
	std::cout << if_not_equal("  Close on exit", entry.close_on_exit,
	                          setup::icon_entry::NoSetting);
	std::cout << if_not_zero("  Hotkey", entry.hotkey);
	std::cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::ini_entry & entry) {
	
	(void)i;
	
	std::cout << " - in " << quoted(entry.inifile);
	std::cout << " set [" << quoted(entry.section) << "] ";
	std::cout << quoted(entry.key) << " = " << quoted(entry.value) << '\n';
	
	print(std::cout, entry, info.header);
	
	std::cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::registry_entry & entry) {
	
	(void)i;
	
	std::cout << " - ";
	if(entry.hive != setup::registry_entry::Unset) {
		std::cout << entry.hive << '\\';
	}
	std::cout << quoted(entry.key);
	std::cout << '\n' << "  ";
	if(entry.name.empty()) {
		std::cout << "(default)";
	} else {
		std::cout << quoted(entry.name);
	}
	if(!entry.value.empty()) {
		std::cout << " = " << quoted(entry.value);
	}
	if(entry.type != setup::registry_entry::None) {
		std::cout << " (" << color::cyan << entry.type << color::reset << ')';
	}
	std::cout << '\n';
	
	print(std::cout, entry, info.header);
	
	if(!entry.permissions.empty()) {
		std::cout << "  Permissions: " << entry.permissions.length() << " bytes";
	}
	std::cout << if_not_equal("  Permission entry", entry.permission, -1);
	std::cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::delete_entry & entry) {
	
	(void)i;
	
	std::cout << " - " << quoted(entry.name)
	     << " (" << color::cyan << entry.type << color::reset << ')' << '\n';
	
	print(std::cout, entry, info.header);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::run_entry & entry) {
	
	(void)i;
	
	std::cout << " - " << quoted(entry.name) << ':' << '\n';
	std::cout << if_not_empty("  Parameters", entry.parameters);
	std::cout << if_not_empty("  Working directory", entry.working_dir);
	std::cout << if_not_empty("  Run once id", entry.run_once_id);
	std::cout << if_not_empty("  Status message", entry.status_message);
	std::cout << if_not_empty("  Verb", entry.verb);
	std::cout << if_not_empty("  Description", entry.verb);
	
	print(std::cout, entry, info.header);
	
	std::cout << if_not_equal("  Show command", entry.show_command, 1);
	std::cout << if_not_equal("  Wait", entry.wait, setup::run_entry::WaitUntilTerminated);
	std::cout << if_not_zero("  Options", entry.options);
}

static void print_entry(const setup::info & info, size_t i,
                        const setup::data_entry & entry) {
	
	(void)info;
	
	std::cout << " - " << "File location #" << i << ':' << '\n';
	std::cout << if_not_zero("  First slice", entry.chunk.first_slice);
	std::cout << if_not_equal("  Last slice", entry.chunk.last_slice,
	                          entry.chunk.first_slice);
	std::cout << "  Chunk: offset " << color::cyan << print_hex(entry.chunk.offset)
	          << color::reset << " size " << color::cyan << print_hex(entry.chunk.size)
	          << color::reset << '\n';
	std::cout << if_not_zero("  File offset", print_hex(entry.file.offset));
	std::cout << if_not_zero("  File size", print_bytes(entry.file.size));
	
	std::cout << "  Checksum: " << entry.file.checksum << '\n';
	
	std::tm t = util::format_time(entry.timestamp);
	
	bool isUTC = ((entry.options & setup::data_entry::TimeStampInUTC) != 0);
	std::cout << "  Timestamp: " << color::cyan << (t.tm_year + 1900)
	          << '-' << std::setfill('0') << std::setw(2) << (t.tm_mon + 1)
	          << '-' << std::setfill('0') << std::setw(2) << t.tm_mday
	          << ' ' << std::setfill(' ') << std::setw(2) << t.tm_hour
	          << ':' << std::setfill('0') << std::setw(2) << t.tm_min
	          << ':' << std::setfill('0') << std::setw(2) << t.tm_sec
	          << color::reset << " +" << entry.timestamp_nsec
	          << (isUTC ? " (UTC)" : " (local)")
	          << '\n';
	
	setup::data_entry::flags options = entry.options;
	options &= ~setup::data_entry::VersionInfoNotValid;
	std::cout << if_not_zero("  Options", options);
	
	if(entry.options & setup::data_entry::VersionInfoValid) {
		std::cout << "  File version: " << ((entry.file_version >> 48) & 0xffff) << '.'
		                                << ((entry.file_version >> 32) & 0xffff) << '.'
		                                << ((entry.file_version >> 16) & 0xffff) << '.'
		                                << ((entry.file_version >>  0) & 0xffff) << '\n';
	}
}

template <class Entry>
static void print_entries(const setup::info & info, const std::vector<Entry> & entries,
                          const std::string & name) {
	
	if(entries.empty()) {
		return;
	}
	
	std::cout << '\n' << name << ":\n";
	for(size_t i = 0; i < entries.size(); i++) {
		print_entry(info, i, entries[i]);
	}
}

static void print_header(const setup::header & header) {
	
	std::cout << if_not_empty("App name", header.app_name);
	std::cout << if_not_empty("App ver name", header.app_versioned_name);
	std::cout << if_not_empty("App id", header.app_id);
	std::cout << if_not_empty("Copyright", header.app_copyright);
	std::cout << if_not_empty("Publisher", header.app_publisher);
	std::cout << if_not_empty("Publisher URL", header.app_publisher_url);
	std::cout << if_not_empty("Support phone", header.app_support_phone);
	std::cout << if_not_empty("Support URL", header.app_support_url);
	std::cout << if_not_empty("Updates URL", header.app_updates_url);
	std::cout << if_not_empty("Version", header.app_version);
	std::cout << if_not_empty("Default dir name", header.default_dir_name);
	std::cout << if_not_empty("Default group name", header.default_group_name);
	std::cout << if_not_empty("Uninstall icon name", header.uninstall_icon_name);
	std::cout << if_not_empty("Base filename", header.base_filename);
	std::cout << if_not_empty("Uninstall files dir", header.uninstall_files_dir);
	std::cout << if_not_empty("Uninstall display name", header.uninstall_name);
	std::cout << if_not_empty("Uninstall display icon", header.uninstall_icon);
	std::cout << if_not_empty("App mutex", header.app_mutex);
	std::cout << if_not_empty("Default user name", header.default_user_name);
	std::cout << if_not_empty("Default user org", header.default_user_organisation);
	std::cout << if_not_empty("Default user serial", header.default_serial);
	std::cout << if_not_empty("Readme", header.app_readme_file);
	std::cout << if_not_empty("Contact", header.app_contact);
	std::cout << if_not_empty("Comments", header.app_comments);
	std::cout << if_not_empty("Modify path", header.app_modify_path);
	std::cout << if_not_empty("Uninstall reg key", header.create_uninstall_registry_key);
	std::cout << if_not_empty("Uninstallable", header.uninstallable);
	std::cout << if_not_empty("License", header.license_text);
	std::cout << if_not_empty("Info before text", header.info_before);
	std::cout << if_not_empty("Info after text", header.info_after);
	std::cout << if_not_empty("Uninstaller signature", header.uninstaller_signature);
	std::cout << if_not_empty("Compiled code", header.compiled_code);
	
	std::cout << if_not_zero("Lead bytes", header.lead_bytes);
	
	std::cout << if_not_zero("Language entries", header.language_count);
	std::cout << if_not_zero("Custom message entries", header.message_count);
	std::cout << if_not_zero("Permission entries", header.permission_count);
	std::cout << if_not_zero("Type entries", header.type_count);
	std::cout << if_not_zero("Component entries", header.component_count);
	std::cout << if_not_zero("Task entries", header.task_count);
	std::cout << if_not_zero("Dir entries", header.directory_count);
	std::cout << if_not_zero("File entries", header.file_count);
	std::cout << if_not_zero("File location entries", header.data_entry_count);
	std::cout << if_not_zero("Icon entries", header.icon_count);
	std::cout << if_not_zero("Ini entries", header.ini_entry_count);
	std::cout << if_not_zero("Registry entries", header.registry_entry_count);
	std::cout << if_not_zero("Delete entries", header.delete_entry_count);
	std::cout << if_not_zero("Uninstall delete entries", header.uninstall_delete_entry_count);
	std::cout << if_not_zero("Run entries", header.run_entry_count);
	std::cout << if_not_zero("Uninstall run entries", header.uninstall_run_entry_count);
	
	std::cout << if_not_equal("Min version", header.winver.begin,
	                          setup::windows_version::none);
	std::cout << if_not_equal("Only below version", header.winver.end,
	                          setup::windows_version::none);
	
	std::cout << std::hex;
	std::cout << if_not_zero("Back color", header.back_color);
	std::cout << if_not_zero("Back color2", header.back_color2);
	std::cout << if_not_zero("Wizard image back color", header.image_back_color);
	std::cout << if_not_zero("Wizard small image back color",
	                         header.small_image_back_color);
	std::cout << std::dec;
	
	if(header.options & (setup::header::Password | setup::header::EncryptionUsed)) {
		std::cout << "Password: " << color::cyan << header.password << color::reset << '\n';
		if(!header.password_salt.empty()) {
			std::cout << "Password salt: " << color::cyan
			          << print_hex(header.password_salt) << color::reset << '\n';
		}
	}
	
	std::cout << if_not_zero("Extra disk space required", header.extra_disk_space_required);
	std::cout << if_not_zero("Slices per disk", header.slices_per_disk);
	
	std::cout << if_not_equal("Install mode", header.install_mode,
	                          setup::header::NormalInstallMode);
	std::cout << "Uninstall log mode: " << color::cyan << header.uninstall_log_mode
	          << color::reset << '\n';
	std::cout << "Uninstall style: " << color::cyan << header.uninstall_style
	          << color::reset << '\n';
	std::cout << "Dir exists warning: " << color::cyan << header.dir_exists_warning
	          << color::reset << '\n';
	std::cout << if_not_equal("Privileges required", header.privileges_required,
	                                                 setup::header::NoPrivileges);
	std::cout << "Show language dialog: " << color::cyan << header.show_language_dialog
	          << color::reset << '\n';
	std::cout << if_not_equal("Language detection", header.language_detection,
	                          setup::header::NoLanguageDetection);
	std::cout << "Compression: " << color::cyan << header.compression
	          << color::reset << '\n';
	std::cout << "Architectures allowed: " << color::cyan << header.architectures_allowed
	          << color::reset << '\n';
	std::cout << "Architectures installed in 64-bit mode: " << color::cyan
	          << header.architectures_installed_in_64bit_mode << color::reset << '\n';
	
	if(header.options & setup::header::SignedUninstaller) {
		std::cout << if_not_zero("Size before signing uninstaller",
		                         header.signed_uninstaller_original_size);
		std::cout << if_not_zero("Uninstaller header checksum",
		                         header.signed_uninstaller_header_checksum);
	}
	
	std::cout << "Disable dir page: " << color::cyan << header.disable_dir_page
	          << color::reset << '\n';
	std::cout << "Disable program group page: " << color::cyan
	          << header.disable_program_group_page << color::reset << '\n';
	
	std::cout << if_not_zero("Uninstall display size", header.uninstall_display_size);
	
	std::cout << "Options: " << color::green << header.options << color::reset << '\n';
	
	std::cout << color::reset;
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
	
	for(size_t i = 0; i < size_t(boost::size(magic_numbers)); i++) {
		
		size_t n = strlen(magic_numbers[i][0]);
		
		if(!data.compare(0, n, magic_numbers[i][0], n)) {
			return magic_numbers[i][1];
		}
	}
	
	return "bin";
}

static void print_aux(const setup::info & info) {
	
	if(info.wizard_images.empty() && info.wizard_images_small.empty()
	   && info.decompressor_dll.empty()) {
		return;
	}
	
	std::cout << '\n';
	
	for(size_t i = 0; i < info.wizard_images.size(); i++) {
		std::cout << "Wizard image #" << (i + 1) << ": " << print_bytes(info.wizard_images[i].length())
		          << " (" << guess_extension(info.wizard_images[i]) << ")\n";
	}
	
	for(size_t i = 0; i < info.wizard_images_small.size(); i++) {
		std::cout << "Wizard small image #" << (i + 1) << ": "
		          << print_bytes(info.wizard_images_small[i].length())
		          << " (" << guess_extension(info.wizard_images_small[i]) << ")\n";
	}
	
	if(!info.decompressor_dll.empty()) {
		std::cout << "Decompressor dll: " << print_bytes(info.decompressor_dll.length())
		          << " (" << guess_extension(info.decompressor_dll) << ")\n";
	}
	
}

void print_info(const setup::info & info) {
	
	std::ios_base::fmtflags old = std::cout.flags();
	std::cout << std::boolalpha;
	
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
	
	std::cout.setf(old, std::ios_base::boolalpha);
}

static void dump_headers(std::istream & is, const setup::version & version, const extract_options & o, int i) {
	
	std::string filename;
	{
		std::ostringstream oss;
		oss << "headers" << i << ".bin";
		filename = oss.str();
	}
	
	const char * type = (i == 0 ? "primary" : "secondary");
	if(!o.quiet) {
		std::cout << "Dumping " << type << " setup headers to \""
		          << color::white << filename << color::reset << "\"\n";
	} else if(!o.silent) {
		std::cout << filename << '\n';
	}
	
	fs::path path = o.output_dir / filename;
	util::ofstream ofs;
	try {
		ofs.open(path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
		if(!ofs.is_open()) {
			throw std::exception();
		}
	} catch(...) {
		throw std::runtime_error("Could not open output file \"" + path.string() + '"');
	}
	
	try {
		ofs << stream::block_reader::get(is, version)->rdbuf();
	} catch(const std::exception & e) {
		std::ostringstream oss;
		oss << "Stream error while dumping " << type << " setup headers!\n";
		oss << " ├─ detected setup version: " << version << '\n';
		oss << " └─ error reason: " << e.what();
		throw format_error(oss.str());
	}
	
}

void dump_headers(std::istream & is, const loader::offsets & offsets, const extract_options & o) {
	
	setup::version version;
	is.seekg(offsets.header_offset);
	version.load(is);
	
	dump_headers(is, version, o, 0);
	dump_headers(is, version, o, 1);
	
}

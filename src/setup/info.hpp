/*
 * Copyright (C) 2011 Daniel Scharrer
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

#ifndef INNOEXTRACT_SETUP_INFO_HPP
#define INNOEXTRACT_SETUP_INFO_HPP

#include <vector>
#include <iosfwd>

#include "setup/header.hpp"
#include "setup/version.hpp"
#include "util/flags.hpp"

namespace setup {

struct component_entry;
struct data_entry;
struct delete_entry;
struct directory_entry;
struct file_entry;
struct icon_entry;
struct ini_entry;
struct language_entry;
struct message_entry;
struct permission_entry;
struct registry_entry;
struct run_entry;
struct task_entry;
struct type_entry;

struct info {
	
	info();
	~info();
	
	FLAGS(entry_types,
		Components,
		DataEntries,
		DeleteEntries,
		UninstallDeleteEntries,
		Directories,
		Files,
		Icons,
		IniEntries,
		Languages,
		Messages,
		Permissions,
		RegistryEntries,
		RunEntries,
		UninstallRunEntries,
		Tasks,
		Types,
		WizardImages,
		DecompressorDll,
		NoSkip
	);
	
	setup::version version;
	
	setup::header header;
	
	std::vector<component_entry> components;
	std::vector<data_entry> data_entries;
	std::vector<delete_entry> delete_entries;
	std::vector<delete_entry> uninstall_delete_entries;
	std::vector<directory_entry> directories;
	std::vector<file_entry> files;
	std::vector<icon_entry> icons;
	std::vector<ini_entry> ini_entries;
	std::vector<language_entry> languages;
	std::vector<message_entry> messages;
	std::vector<permission_entry> permissions;
	std::vector<registry_entry> registry_entries;
	std::vector<run_entry> run_entries;
	std::vector<run_entry> uninstall_run_entries;
	std::vector<task_entry> tasks;
	std::vector<type_entry> types;
	
	std::string wizard_image;
	std::string wizard_image_small;
	
	std::string decompressor_dll;
	
	void load(std::istream & is, entry_types entries = setup::info::entry_types::all());
	
	void load(std::istream & is, entry_types entries, const setup::version & version);
	
};

} // namespace setup

FLAGS_OVERLOADS(setup::info::entry_types)

#endif // INNOEXTRACT_SETUP_INFO_HPP

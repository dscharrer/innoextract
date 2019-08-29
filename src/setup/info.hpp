/*
 * Copyright (C) 2011-2019 Daniel Scharrer
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

/*!
 * \file
 *
 * Central point to load all the different headers in the correct order.
 */
#ifndef INNOEXTRACT_SETUP_INFO_HPP
#define INNOEXTRACT_SETUP_INFO_HPP

#include <vector>
#include <iosfwd>

#include "setup/header.hpp"
#include "setup/version.hpp"
#include "util/encoding.hpp"
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

/*!
 * Class used to hold and load the various \ref setup headers.
 */
struct info {
	
	// Explicit constructor/destructor required to allow forward-declaring entry types
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
		DecryptDll,
		NoSkip,
		NoUnknownVersion
	);
	
	setup::version version;
	
	util::codepage_id codepage;
	
	setup::header header;
	
	std::vector<component_entry>  components;               //! \c Components
	std::vector<data_entry>       data_entries;             //! \c DataEntries
	std::vector<delete_entry>     delete_entries;           //! \c DeleteEntries
	std::vector<delete_entry>     uninstall_delete_entries; //! \c UninstallDeleteEntries
	std::vector<directory_entry>  directories;              //! \c Directories
	std::vector<file_entry>       files;                    //! \c Files
	std::vector<icon_entry>       icons;                    //! \c Icons
	std::vector<ini_entry>        ini_entries;              //! \c IniEntries
	std::vector<language_entry>   languages;                //! \c Languages
	std::vector<message_entry>    messages;                 //! \c Messages
	std::vector<permission_entry> permissions;              //! \c Permissions
	std::vector<registry_entry>   registry_entries;         //! \c RegistryEntries
	std::vector<run_entry>        run_entries;              //! \c RunEntries
	std::vector<run_entry>        uninstall_run_entries;    //! \c UninstallRunEntries
	std::vector<task_entry>       tasks;                    //! \c Tasks
	std::vector<type_entry>       types;                    //! \c Types
	
	//! Images displayed in the installer UI.
	//! Loading enabled by \c WizardImages
	std::vector<std::string> wizard_images;
	std::vector<std::string> wizard_images_small;
	
	//! Contents of the helper DLL used to decompress setup data in some versions.
	//! Loading enabled by \c DecompressorDll
	std::string decompressor_dll;
	
	//! Contents of the helper DLL used to decrypt setup data.
	//! Loading enabled by \c DecryptDll
	std::string decrypt_dll;
	
	/*!
	 * Load setup headers.
	 *
	 * \param is      The input stream to load the setup headers from.
	 *                It must already be positioned at start of \ref setup::version
	 *                identifier whose position is given by
	 *                \ref loader::offsets::header_offset.
	 * \param entries What kinds of entries to load.
	 * \param force_codepage Windows codepage to use for strings in ANSI installers.
	 */
	void load(std::istream & is, entry_types entries, util::codepage_id force_codepage = 0);
	
private:
	
	/*!
	 * Load setup headers for a specific version.
	 *
	 * \param is      The input stream to load the setup headers from.
	 *                It must already be positioned at start of the compressed headers.
	 *                The compressed headers start directly after the \ref setup::version
	 *                identifier whose position is given by
	 *                \ref loader::offsets::header_offset.
	 * \param entries What kinds of entries to load.
	 * \param force_codepage Windows codepage to use for strings in ANSI installers.
	 *
	 * This function does not set the \ref version member.
	 */
	void try_load(std::istream & is, entry_types entries, util::codepage_id force_codepage);
	
	template <class Entry>
	void load_entries(std::istream & is, entry_types entries, size_t count,
	                  std::vector<Entry> & result, entry_types::enum_type entry_type);
	
};

} // namespace setup

FLAGS_OVERLOADS(setup::info::entry_types)

#endif // INNOEXTRACT_SETUP_INFO_HPP

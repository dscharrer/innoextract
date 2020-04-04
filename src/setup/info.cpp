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

#include "setup/info.hpp"

#include <cassert>
#include <istream>
#include <sstream>

#include <boost/foreach.hpp>

#include "setup/component.hpp"
#include "setup/data.hpp"
#include "setup/delete.hpp"
#include "setup/directory.hpp"
#include "setup/file.hpp"
#include "setup/icon.hpp"
#include "setup/ini.hpp"
#include "setup/item.hpp"
#include "setup/language.hpp"
#include "setup/message.hpp"
#include "setup/permission.hpp"
#include "setup/registry.hpp"
#include "setup/run.hpp"
#include "setup/task.hpp"
#include "setup/type.hpp"
#include "stream/block.hpp"
#include "util/fstream.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/output.hpp"

namespace setup {

template <class Entry>
void info::load_entries(std::istream & is, entry_types entries, size_t count,
                        std::vector<Entry> & result, entry_types::enum_type entry_type) {
	
	result.clear();
	if(entries & entry_type) {
		result.resize(count);
		for(size_t i = 0; i < count; i++) {
			result[i].load(is, *this);
		}
	} else {
		for(size_t i = 0; i < count; i++) {
			Entry entry;
			entry.load(is, *this);
		}
	}
}

namespace {

void load_wizard_images(std::istream & is, const setup::version & version,
                        std::vector<std::string> & images, info::entry_types entries) {
	
	size_t count = 1;
	if(version >= INNO_VERSION(5, 6, 0)) {
		count = util::load<boost::uint32_t>(is);
	}
	
	if(entries & (info::WizardImages | info::NoSkip)) {
		images.resize(count);
		for(size_t i = 0; i < count; i++) {
			is >> util::binary_string(images[i]);
		}
		if(version < INNO_VERSION(5, 6, 0) && images[0].empty()) {
			images.clear();
		}
	} else {
		for(size_t i = 0; i < count; i++) {
			util::binary_string::skip(is);
		}
	}
	
}

void load_wizard_and_decompressor(std::istream & is, const setup::version & version,
                                  const setup::header & header,
                                  setup::info & info, info::entry_types entries) {
	
	info.wizard_images.clear();
	info.wizard_images_small.clear();
	
	load_wizard_images(is, version, info.wizard_images, entries);
	
	if(version >= INNO_VERSION(2, 0, 0) || version.is_isx()) {
		load_wizard_images(is, version, info.wizard_images_small, entries);
	}
	
	info.decompressor_dll.clear();
	if(header.compression == stream::BZip2
	   || (header.compression == stream::LZMA1 && version == INNO_VERSION(4, 1, 5))
	   || (header.compression == stream::Zlib && version >= INNO_VERSION(4, 2, 6))) {
		if(entries & (info::DecompressorDll | info::NoSkip)) {
			is >> util::binary_string(info.decompressor_dll);
		} else {
			// decompressor dll - we don't need this
			util::binary_string::skip(is);
		}
	}
	
	info.decrypt_dll.clear();
	if(header.options & header::EncryptionUsed) {
		if(entries & (info::DecryptDll | info::NoSkip)) {
			is >> util::binary_string(info.decrypt_dll);
		} else {
			// decrypt dll - we don't need this
			util::binary_string::skip(is);
		}
	}
	
}

void check_is_end(stream::block_reader::pointer & is, const char * what) {
	is->exceptions(std::ios_base::goodbit);
	char dummy;
	if(!is->get(dummy).eof()) {
		throw std::ios_base::failure(what);
	}
}

} // anonymous namespace

void info::try_load(std::istream & is, entry_types entries, util::codepage_id force_codepage) {
	
	debug("trying to load setup headers for version " << version);
	
	if((entries & (Messages | NoSkip)) || (!version.is_unicode() && !force_codepage)) {
		entries |= Languages;
	}
	
	stream::block_reader::pointer reader = stream::block_reader::get(is, version);
	
	debug("loading main header");
	header.load(*reader, version);
	
	debug("loading languages");
	load_entries(*reader, entries, header.language_count, languages, Languages);
	
	debug("determining encoding");
	if(version.is_unicode()) {
		// Unicode installers are always UTF16-LE, do not allow users to override that.
		codepage = util::cp_utf16le;
	} else if(force_codepage) {
		codepage = force_codepage;
	} else if(languages.empty()) {
		codepage = util::cp_windows1252;
	} else {
		// Non-Unicode installers do not have a defined codepage but instead just assume the
		// codepage of the system the installer is run on.
		// Look at the list of available languages to guess a suitable codepage.
		codepage = languages[0].codepage;
		BOOST_FOREACH(const language_entry & language, languages) {
			if(language.codepage == util::cp_windows1252) {
				codepage = util::cp_windows1252;
				break;
			}
		}
	}
	
	header.decode(codepage);
	BOOST_FOREACH(language_entry & language, languages) {
		language.decode(codepage);
	}
	
	if(version < INNO_VERSION(4, 0, 0)) {
		debug("loading images and plugins");
		load_wizard_and_decompressor(*reader, version, header, *this, entries);
	}
	
	debug("loading messages");
	load_entries(*reader, entries, header.message_count, messages, Messages);
	debug("loading permissions");
	load_entries(*reader, entries, header.permission_count, permissions, Permissions);
	debug("loading types");
	load_entries(*reader, entries, header.type_count, types, Types);
	debug("loading components");
	load_entries(*reader, entries, header.component_count, components, Components);
	debug("loading tasks");
	load_entries(*reader, entries, header.task_count, tasks, Tasks);
	debug("loading directories");
	load_entries(*reader, entries, header.directory_count, directories, Directories);
	debug("loading files");
	load_entries(*reader, entries, header.file_count, files, Files);
	debug("loading icons");
	load_entries(*reader, entries, header.icon_count, icons, Icons);
	debug("loading ini entries");
	load_entries(*reader, entries, header.ini_entry_count, ini_entries, IniEntries);
	debug("loading registry entries");
	load_entries(*reader, entries, header.registry_entry_count, registry_entries, RegistryEntries);
	debug("loading delete entries");
	load_entries(*reader, entries, header.delete_entry_count, delete_entries, DeleteEntries);
	debug("loading uninstall delete entries");
	load_entries(*reader, entries, header.uninstall_delete_entry_count, uninstall_delete_entries,
	             UninstallDeleteEntries);
	debug("loading run entries");
	load_entries(*reader, entries, header.run_entry_count, run_entries, RunEntries);
	debug("loading uninstall run entries");
	load_entries(*reader, entries, header.uninstall_run_entry_count, uninstall_run_entries,
	             UninstallRunEntries);
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		debug("loading images and plugins");
		load_wizard_and_decompressor(*reader, version, header, *this, entries);
	}
	
	// restart the compression stream
	check_is_end(reader, "unknown data at end of primary header stream");
	reader = stream::block_reader::get(is, version);
	
	debug("loading data entries");
	load_entries(*reader, entries, header.data_entry_count, data_entries, DataEntries);
	
	check_is_end(reader, "unknown data at end of secondary header stream");
}

void info::load(std::istream & is, entry_types entries, util::codepage_id force_codepage) {
	
	version.load(is);
	
	if(!version.known) {
		if(entries & NoUnknownVersion) {
			std::ostringstream oss;
			oss << "Unexpected setup data version: " << version;
			throw std::runtime_error(oss.str());
		}
		log_warning << "Unexpected setup data version: "
		            << color::white << version << color::reset;
	}
	
	version_constant listed_version = version.value;
	
	// Some setup versions didn't increment the data version number when they should have.
	// To work around this, we try to parse the headers for all data versions and use the first
	// version that parses without warnings or errors.
	bool ambiguous = !version.known || version.is_ambiguous();
	if(version.is_ambiguous()) {
		// Force parsing all headers so that we don't miss any errors.
		entries |= NoSkip;
	}
	
	bool parsed_without_errors = false;
	std::streampos start = is.tellg();
	for(;;) {
		
		warning_suppressor warnings;
		
		try {
			
			// Try to parse headers for this version
			try_load(is, entries, force_codepage);
			
			if(warnings) {
				// Parsed without errors but with warnings - try other versions first
				if(!parsed_without_errors) {
					listed_version = version.value;
					parsed_without_errors = true;
				}
				throw std::exception();
			}
			
			warnings.flush();
			return;
			
		} catch(...) {
			
			is.clear();
			is.seekg(start);
			
			version_constant next_version = version.next();
			
			if(!ambiguous || !next_version) {
				if(version.value != listed_version) {
					// Rewind to a previous version that had better results and report those
					version.value = listed_version;
					warnings.restore();
					try_load(is, entries, force_codepage);
				} else {
					// Otherwise. report results for the current version
					warnings.flush();
					if(!parsed_without_errors) {
						throw;
					}
				}
				return;
			}
			
			// Retry with the next version
			version.value = next_version;
			ambiguous = version.is_ambiguous();
			
		}
		
	}
	
}

info::info() : codepage(0) { }
info::~info() { }

} // namespace setup

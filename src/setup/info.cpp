/*
 * Copyright (C) 2011-2018 Daniel Scharrer
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
#include "util/load.hpp"
#include "util/log.hpp"

namespace setup {

namespace {

struct no_arg { };

template <class Entry, class Arg>
void load_entry(std::istream & is, const setup::version & version,
                       Entry & entity, Arg arg) {
	entity.load(is, version, arg);
}
template <class Entry>
void load_entry(std::istream & is, const setup::version & version,
                                    Entry & entity, no_arg arg) {
	(void)arg;
	entity.load(is, version);
}

template <class Entry, class Arg>
void load_entries(std::istream & is, const setup::version & version,
                  info::entry_types entry_types, size_t count,
                  std::vector<Entry> & entries, info::entry_types::enum_type entry_type,
                  Arg arg = Arg()) {
	
	entries.clear();
	if(entry_types & entry_type) {
		entries.resize(count);
		for(size_t i = 0; i < count; i++) {
			Entry & entry = entries[i];
			load_entry(is, version, entry, arg);
		}
	} else {
		for(size_t i = 0; i < count; i++) {
			Entry entry;
			load_entry(is, version, entry, arg);
		}
	}
}

template <class Entry>
void load_entries(std::istream & is, const setup::version & version,
                  info::entry_types entry_types, size_t count,
                  std::vector<Entry> & entries, info::entry_types::enum_type entry_type) {
	load_entries<Entry, no_arg>(is, version, entry_types, count, entries, entry_type);
}

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
	
	if(version >= INNO_VERSION(2, 0, 0)) {
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

void info::load(std::istream & ifs, entry_types e, const setup::version & v) {
	
	if(e & (Messages | NoSkip)) {
		e |= Languages;
	}
	
	stream::block_reader::pointer is = stream::block_reader::get(ifs, v);
	
	header.load(*is, v);
	
	load_entries(*is, v, e, header.language_count, languages, Languages);
	
	if(v < INNO_VERSION(4, 0, 0)) {
		load_wizard_and_decompressor(*is, v, header, *this, e);
	}
	
	load_entries(*is, v, e, header.message_count, messages, Messages, languages);
	load_entries(*is, v, e, header.permission_count, permissions, Permissions);
	load_entries(*is, v, e, header.type_count, types, Types);
	load_entries(*is, v, e, header.component_count, components, Components);
	load_entries(*is, v, e, header.task_count, tasks, Tasks);
	load_entries(*is, v, e, header.directory_count, directories, Directories);
	load_entries(*is, v, e, header.file_count, files, Files);
	load_entries(*is, v, e, header.icon_count, icons, Icons);
	load_entries(*is, v, e, header.ini_entry_count, ini_entries, IniEntries);
	load_entries(*is, v, e, header.registry_entry_count, registry_entries, RegistryEntries);
	load_entries(*is, v, e, header.delete_entry_count, delete_entries, DeleteEntries);
	load_entries(*is, v, e, header.uninstall_delete_entry_count, uninstall_delete_entries,
	             UninstallDeleteEntries);
	load_entries(*is, v, e, header.run_entry_count, run_entries, RunEntries);
	load_entries(*is, v, e, header.uninstall_run_entry_count, uninstall_run_entries,
	             UninstallRunEntries);
	
	if(v >= INNO_VERSION(4, 0, 0)) {
		load_wizard_and_decompressor(*is, v, header, *this, e);
	}
	
	// restart the compression stream
	check_is_end(is, "unknown data at end of primary header stream");
	is = stream::block_reader::get(ifs, v);
	
	load_entries(*is, v, e, header.data_entry_count, data_entries, DataEntries);
	
	check_is_end(is, "unknown data at end of secondary header stream");
}

void info::load(std::istream & is, entry_types entries) {
	
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
	// To work around this, we try to parse the headers for both data versions.
	bool ambiguous = version.is_ambiguous();
	if(ambiguous) {
		// Force parsing all headers so that we don't miss any errors.
		entries |= NoSkip;
	}
	if(!version.known || ambiguous) {
		std::streampos start = is.tellg();
		try {
			load(is, entries, version);
			return;
		} catch(...) {
			version.value = version.next();
			if(!version) {
				version.value = listed_version;
				throw;
			}
			is.clear();
			is.seekg(start);
		}
	}
	
	try {
		load(is, entries, version);
	} catch(...) {
		version.value = listed_version;
		throw;
	}
}

info::info() { }
info::~info() { }

} // namespace setup

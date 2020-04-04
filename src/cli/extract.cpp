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

#include "cli/extract.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <limits>

#include <boost/foreach.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range/size.hpp>

#include <boost/version.hpp>
#if BOOST_VERSION >= 104800
#include <boost/container/flat_map.hpp>
#endif

#include "cli/debug.hpp"
#include "cli/gog.hpp"
#include "cli/goggalaxy.hpp"

#include "crypto/checksum.hpp"
#include "crypto/hasher.hpp"

#include "loader/offsets.hpp"

#include "setup/data.hpp"
#include "setup/directory.hpp"
#include "setup/expression.hpp"
#include "setup/file.hpp"
#include "setup/info.hpp"
#include "setup/language.hpp"

#include "stream/chunk.hpp"
#include "stream/file.hpp"
#include "stream/slice.hpp"

#include "util/boostfs_compat.hpp"
#include "util/console.hpp"
#include "util/encoding.hpp"
#include "util/fstream.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/output.hpp"
#include "util/time.hpp"

namespace fs = boost::filesystem;

namespace {

template <typename Entry>
class processed_item {
	
	std::string path_;
	const Entry * entry_;
	
public:
	
	processed_item(const std::string & path, const Entry * entry)
		: path_(path), entry_(entry) { }
	
	bool has_entry() const { return entry_ != NULL; }
	const Entry & entry() const { return *entry_; }
	const std::string & path() const { return path_; }
	
	void set_entry(const Entry * entry) { entry_ = entry; }
	void set_path(const std::string & path) { path_ = path; }
	
};

class processed_file : public processed_item<setup::file_entry> {
	
public:
	
	processed_file(const setup::file_entry * entry, const std::string & path)
		: processed_item<setup::file_entry>(path, entry) { }
	
	bool is_multipart() const { return !entry().additional_locations.empty(); }
	
};

class processed_directory : public processed_item<setup::directory_entry> {
	
	bool implied_;
	
public:
	
	explicit processed_directory(const std::string & path)
		: processed_item<setup::directory_entry>(path, NULL), implied_(false) { }
	
	bool implied() const { return implied_; }
	
	void set_implied(bool implied) { implied_ = implied; }
	
};

class file_output : private boost::noncopyable {
	
	fs::path path_;
	const processed_file * file_;
	util::fstream stream_;
	
	crypto::hasher checksum_;
	boost::uint64_t checksum_position_;
	
	boost::uint64_t position_;
	boost::uint64_t total_written_;
	
	bool write_;
	
public:
	
	explicit file_output(const fs::path & dir, const processed_file * f, bool write)
		: path_(dir / f->path())
		, file_(f)
		, checksum_(f->entry().checksum.type)
		, checksum_position_(f->entry().checksum.type == crypto::None ? boost::uint64_t(-1) : 0)
		, position_(0)
		, total_written_(0)
		, write_(write)
	{
		if(write_) {
			try {
				std::ios_base::openmode flags = std::ios_base::out | std::ios_base::binary | std::ios_base::trunc;
				if(file_->is_multipart()) {
					flags |= std::ios_base::in;
				}
				stream_.open(path_, flags);
				if(!stream_.is_open()) {
					throw std::exception();
				}
			} catch(...) {
				throw std::runtime_error("Coul not open output file \"" + path_.string() + '"');
			}
		}
	}
	
	bool write(const char * data, size_t n) {
		
		if(write_) {
			stream_.write(data, std::streamsize(n));
		}
		
		if(checksum_position_ == position_) {
			checksum_.update(data, n);
			checksum_position_ += n;
		}
		
		position_ += n;
		total_written_ += n;
		
		return !write_ || !stream_.fail();
	}
	
	void seek(boost::uint64_t new_position) {
		
		if(new_position == position_) {
			return;
		}
		
		debug("seeking output from " << print_hex(position_) << " to " << print_hex(new_position));
		
		if(!write_) {
			position_ = new_position;
			return;
		}
		
		const boost::uint64_t max = boost::uint64_t(std::numeric_limits<util::fstream::off_type>::max() / 4);
		
		if(new_position <= max) {
			stream_.seekp(util::fstream::off_type(new_position), std::ios_base::beg);
		} else {
			util::fstream::off_type sign = (new_position > position_) ? 1 : -1;
			boost::uint64_t diff = (new_position > position_) ? new_position - position_ : position_ - new_position;
			while(diff > 0) {
				stream_.seekp(sign * util::fstream::off_type(std::min(diff, max)), std::ios_base::cur);
				diff -= std::min(diff, max);
			}
		}
		
		position_ = new_position;
		
	}
	
	void close() {
		
		if(write_) {
			stream_.close();
		}
		
	}
	
	const fs::path & path() const { return path_; }
	const processed_file * file() const { return file_; }
	
	bool is_complete() const {
		return total_written_ == file_->entry().size;
	}
	
	bool has_checksum() const {
		return checksum_position_ == file_->entry().size;
	}
	
	bool calculate_checksum() {
		
		if(has_checksum()) {
			return true;
		}
		
		if(!write_) {
			return false;
		}
		
		debug("calculating output checksum for " << path_);
		
		const boost::uint64_t max = boost::uint64_t(std::numeric_limits<util::fstream::off_type>::max() / 4);
		
		boost::uint64_t diff = checksum_position_;
		stream_.seekg(util::fstream::off_type(std::min(diff, max)), std::ios_base::beg);
		diff -= std::min(diff, max);
		while(diff > 0) {
			stream_.seekg(util::fstream::off_type(std::min(diff, max)), std::ios_base::cur);
			diff -= std::min(diff, max);
		}
		
		while(!stream_.eof()) {
			char buffer[8192];
			std::streamsize n = stream_.read(buffer, sizeof(buffer)).gcount();
			checksum_.update(buffer, size_t(n));
			checksum_position_ += boost::uint64_t(n);
		}
		
		if(!has_checksum()) {
			log_warning << "Could not read back " << path_ << " to calculate output checksum for multi-part file";
			return false;
		}
		
		return true;
	}
	
	crypto::checksum checksum() {
		return checksum_.finalize();
	}
	
};

class path_filter {
	
	typedef std::pair<bool, std::string> Filter;
	std::vector<Filter> includes;
	
public:
	
	explicit path_filter(const extract_options & o) {
		BOOST_FOREACH(const std::string & include, o.include) {
			if(!include.empty() && include[0] == setup::path_sep) {
				includes.push_back(Filter(true, boost::to_lower_copy(include) + setup::path_sep));
			} else {
				includes.push_back(Filter(false, setup::path_sep + boost::to_lower_copy(include)
				                                 + setup::path_sep));
			}
		}
	}
	
	bool match(const std::string & path) const {
		
		if(includes.empty()) {
			return true;
		}
		
		BOOST_FOREACH(const Filter & i, includes) {
			if(i.first) {
				if(!i.second.compare(1, i.second.size() - 1,
				                     path + setup::path_sep, 0, i.second.size() - 1)) {
					return true;
				}
			} else {
				if((setup::path_sep + path + setup::path_sep).find(i.second) != std::string::npos) {
					return true;
				}
			}
		}
		
		return false;
	}
	
};

void print_filter_info(const setup::item & item, bool temp) {
	
	bool first = true;
	
	if(!item.languages.empty()) {
		std::cout << " [";
		first = false;
		std::cout << color::green << item.languages << color::reset;
	}
	
	if(temp) {
		std::cout << (first ? " [" : ", ");
		first = false;
		std::cout << color::cyan << "temp" << color::reset;
		
	}
	
	if(!first) {
		std::cout << "]";
	}
	
}

void print_filter_info(const setup::file_entry & file) {
	bool is_temp = !!(file.options & setup::file_entry::DeleteAfterInstall);
	print_filter_info(file, is_temp);
}

void print_filter_info(const setup::directory_entry & dir) {
	bool is_temp = !!(dir.options & setup::directory_entry::DeleteAfterInstall);
	print_filter_info(dir, is_temp);
}

void print_size_info(const stream::file & file, boost::uint64_t size) {
	
	if(logger::debug) {
		std::cout << " @ " << print_hex(file.offset);
	}
	
	std::cout << " (" << color::dim_cyan << print_bytes(size ? size : file.size) << color::reset << ")";
}

void print_checksum_info(const stream::file & file, const crypto::checksum * checksum) {
	
	if(!checksum || checksum->type == crypto::None) {
		checksum = &file.checksum;
	}
	
	std::cout << color::dim_magenta << *checksum << color::reset;
}

bool prompt_overwrite() {
	return true; // TODO the user always overwrites
}

const char * handle_collision(const setup::file_entry & oldfile, const setup::data_entry & olddata,
                              const setup::file_entry & newfile, const setup::data_entry & newdata) {
	
	bool allow_timestamp = true;
	
	if(!(newfile.options & setup::file_entry::IgnoreVersion)) {
		
		bool version_info_valid = !!(newdata.options & setup::data_entry::VersionInfoValid);
		
		if(olddata.options & setup::data_entry::VersionInfoValid) {
			allow_timestamp = false;
			
			if(!version_info_valid || olddata.file_version > newdata.file_version) {
				if(!(newfile.options & setup::file_entry::PromptIfOlder) || !prompt_overwrite()) {
					return "old version";
				}
			} else if(newdata.file_version == olddata.file_version
				   && !(newfile.options & setup::file_entry::OverwriteSameVersion)) {
				
				if((newfile.options & setup::file_entry::ReplaceSameVersionIfContentsDiffer)
				   && olddata.file.checksum == newdata.file.checksum) {
					return "duplicate (checksum)";
				}
				
				if(!(newfile.options & setup::file_entry::CompareTimeStamp)) {
					return "duplicate (version)";
				}
				
				allow_timestamp = true;
			}
			
		} else if(version_info_valid) {
			allow_timestamp = false;
		}
		
	}
	
	if(allow_timestamp && (newfile.options & setup::file_entry::CompareTimeStamp)) {
		
		if(newdata.timestamp == olddata.timestamp
		   && newdata.timestamp_nsec == olddata.timestamp_nsec) {
			return "duplicate (modification time)";
		}
		
		
		if(newdata.timestamp < olddata.timestamp
		   || (newdata.timestamp == olddata.timestamp
		       && newdata.timestamp_nsec < olddata.timestamp_nsec)) {
			if(!(newfile.options & setup::file_entry::PromptIfOlder) || !prompt_overwrite()) {
				return "old version (modification time)";
			}
		}
		
	}
	
	if((newfile.options & setup::file_entry::ConfirmOverwrite) && !prompt_overwrite()) {
		return "user chose not to overwrite";
	}
	
	if(oldfile.attributes != boost::uint32_t(-1)
	   && (oldfile.attributes & setup::file_entry::ReadOnly) != 0) {
		if(!(newfile.options & setup::file_entry::OverwriteReadOnly) && !prompt_overwrite()) {
			return "user chose not to overwrite read-only file";
		}
	}
	
	return NULL; // overwrite old file
}

typedef boost::unordered_map<std::string, processed_file> FilesMap;
#if BOOST_VERSION >= 104800
typedef boost::container::flat_map<std::string, processed_directory> DirectoriesMap;
#else
typedef std::map<std::string, processed_directory> DirectoriesMap;
#endif
typedef boost::unordered_map<std::string, std::vector<processed_file> > CollisionMap;

std::string parent_dir(const std::string & path) {
	
	size_t pos = path.find_last_of(setup::path_sep);
	if(pos == std::string::npos) {
		return std::string();
	}
	
	return path.substr(0, pos);
}

bool insert_dirs(DirectoriesMap & processed_directories, const path_filter & includes,
                 const std::string & internal_path, std::string & path, bool implied) {
	
	std::string dir = parent_dir(path);
	std::string internal_dir = parent_dir(internal_path);
	
	if(internal_dir.empty()) {
		return false;
	}
	
	if(implied || includes.match(internal_dir)) {
		
		std::pair<DirectoriesMap::iterator, bool> existing = processed_directories.insert(
			std::make_pair(internal_dir, processed_directory(dir))
		);
		
		if(implied) {
			existing.first->second.set_implied(true);
		}
		
		if(!existing.second) {
			if(existing.first->second.path() != dir) {
				// Existing dir case differs, fix path
				if(existing.first->second.path().length() == dir.length()) {
					path.replace(0, dir.length(), existing.first->second.path());
				} else {
					path = existing.first->second.path() + path.substr(dir.length());
				}
				return true;
			} else {
				return false;
			}
		}
		
		implied = true;
	}
	
	size_t oldlength = dir.length();
	if(insert_dirs(processed_directories, includes, internal_dir, dir, implied)) {
		// Existing dir case differs, fix path
		if(dir.length() == oldlength) {
			path.replace(0, dir.length(), dir);
		} else {
			path = dir + path.substr(oldlength);
		}
		// Also fix previously inserted directory
		DirectoriesMap::iterator inserted = processed_directories.find(internal_dir);
		if(inserted != processed_directories.end()) {
			inserted->second.set_path(dir);
		}
		return true;
	}
	
	return false;
}

bool rename_collision(const extract_options & o, FilesMap & processed_files, const std::string & path,
                      const processed_file & other, bool common_component, bool common_language,
                      bool common_arch, bool first) {
	
	const setup::file_entry & file = other.entry();
	
	bool require_number_suffix = !first || (o.collisions == RenameAllCollisions);
	std::ostringstream oss;
	const setup::file_entry::flags arch_flags = setup::file_entry::Bits32 | setup::file_entry::Bits64;
	
	if(!common_component && !file.components.empty()) {
		if(setup::is_simple_expression(file.components)) {
			require_number_suffix = false;
			oss << '#' << file.components;
		}
	}
	if(!common_language && !file.languages.empty()) {
		if(setup::is_simple_expression(file.languages)) {
			require_number_suffix = false;
			if(file.languages != o.default_language) {
				oss << '@' << file.languages;
			}
		}
	}
	if(!common_arch && (file.options & arch_flags) == setup::file_entry::Bits32) {
		require_number_suffix = false;
		oss << "@32bit";
	} else if(!common_arch && (file.options & arch_flags) == setup::file_entry::Bits64) {
		require_number_suffix = false;
		oss << "@64bit";
	}
	
	size_t i = 0;
	std::string suffix = oss.str();
	if(require_number_suffix) {
		oss << '$' << i++;
	}
	for(;;) {
		std::pair<FilesMap::iterator, bool> insertion = processed_files.insert(std::make_pair(
			path + oss.str(), processed_file(&file, other.path() + oss.str())
		));
		if(insertion.second) {
			// Found an available name and inserted
			return true;
		}
		if(&insertion.first->second.entry() == &file) {
			// File already has the desired name, abort
			return false;
		}
		oss.str(suffix);
		oss << '$' << i++;
	}
	
}

void rename_collisions(const extract_options & o, FilesMap & processed_files,
                       const CollisionMap & collisions) {
	
	BOOST_FOREACH(const CollisionMap::value_type & collision, collisions) {
		
		const std::string & path = collision.first;
		
		const processed_file & base = processed_files.find(path)->second;
		const setup::file_entry & file = base.entry();
		const setup::file_entry::flags arch_flags = setup::file_entry::Bits32 | setup::file_entry::Bits64;
		
		bool common_component = true;
		bool common_language = true;
		bool common_arch = true;
		BOOST_FOREACH(const processed_file & other, collision.second) {
			common_component = common_component && other.entry().components == file.components;
			common_language = common_language && other.entry().languages == file.languages;
			common_arch = common_arch && (other.entry().options & arch_flags) == (file.options & arch_flags);
		}
		
		bool ignore_component = common_component || o.collisions != RenameAllCollisions;
		if(rename_collision(o, processed_files, path, base,
		                    ignore_component, common_language, common_arch, true)) {
			processed_files.erase(path);
		}
		
		BOOST_FOREACH(const processed_file & other, collision.second) {
			rename_collision(o, processed_files, path, other,
			                 common_component, common_language, common_arch, false);
		}
		
	}
}

bool print_file_info(const extract_options & o, const setup::info & info) {
	
	if(!o.quiet) {
		const std::string & name = info.header.app_versioned_name.empty()
		                           ? info.header.app_name : info.header.app_versioned_name;
		const char * verb = "Inspecting";
		if(o.extract) {
			verb = "Extracting";
		} else if(o.test) {
			verb = "Testing";
		} else if(o.list) {
			verb = "Listing";
		}
		std::cout << verb << " \"" << color::green << name << color::reset
		          << "\" - setup data version " << color::white << info.version << color::reset
		          << std::endl;
	}
	
	#ifdef DEBUG
	if(logger::debug) {
		std::cout << '\n';
		print_info(info);
		std::cout << '\n';
	}
	#endif
	
	bool multiple_sections = (o.list_languages + o.gog_game_id + o.list + o.show_password > 1);
	if(!o.quiet && multiple_sections) {
		std::cout << '\n';
	}
	
	if(o.list_languages) {
		if(o.silent) {
			BOOST_FOREACH(const setup::language_entry & language, info.languages) {
				std::cout << language.name <<' ' << language.language_name << '\n';
			}
		} else {
			if(multiple_sections) {
				std::cout << "Languages:\n";
			}
			BOOST_FOREACH(const setup::language_entry & language, info.languages) {
				std::cout << " - " << color::green << language.name << color::reset;
				if(!language.language_name.empty()) {
					std::cout << ": " << color::white << language.language_name << color::reset;
				}
				std::cout << '\n';
			}
			if(info.languages.empty()) {
				std::cout << " (none)\n";
			}
		}
		if((o.silent || !o.quiet) && multiple_sections) {
			std::cout << '\n';
		}
	}
	
	if(o.gog_game_id) {
		std::string id = gog::get_game_id(info);
		if(id.empty()) {
			if(!o.quiet) {
				std::cout << "No GOG.com game ID found!\n";
			}
		} else if(!o.silent) {
			std::cout << "GOG.com game ID is " << color::cyan << id << color::reset << '\n';
		} else {
			std::cout << id << '\n';
		}
		if((o.silent || !o.quiet) && multiple_sections) {
			std::cout << '\n';
		}
	}
	
	if(o.show_password) {
		if(info.header.options & setup::header::Password) {
			if(o.silent) {
				std::cout << info.header.password << '\n';
			} else {
				std::cout << "Password hash: " << color::yellow << info.header.password << color::reset << '\n';
			}
			if(o.silent) {
				std::cout << print_hex(info.header.password_salt) << '\n';
			} else if(!info.header.password_salt.empty()) {
				std::cout << "Password salt: " << color::yellow
				          << print_hex(info.header.password_salt) << color::reset;
				if(!o.quiet) {
					std::cout << " (hex bytes, prepended to password)";
				}
				std::cout << '\n';
			}
			if(o.silent) {
				std::cout << util::encoding_name(info.codepage) << '\n';
			} else {
				std::cout << "Password encoding: " << color::yellow
				          << util::encoding_name(info.codepage) << color::reset << '\n';
			}
		} else if(!o.quiet) {
			std::cout << "Setup is not passworded!\n";
		}
		if((o.silent || !o.quiet) && multiple_sections) {
			std::cout << '\n';
		}
	}
	
	return multiple_sections;
}

struct processed_entries {
	
	FilesMap files;
	
	DirectoriesMap directories;
	
};

processed_entries filter_entries(const extract_options & o, const setup::info & info) {
	
	processed_entries processed;
	
	#if BOOST_VERSION >= 105000
	processed.files.reserve(info.files.size());
	#endif
	
	#if BOOST_VERSION >= 104800
	processed.directories.reserve(info.directories.size()
	                              + size_t(std::log(double(info.files.size()))));
	#endif
	
	CollisionMap collisions;
	
	path_filter includes(o);
	
	// Filter the directories to be created
	BOOST_FOREACH(const setup::directory_entry & directory, info.directories) {
		
		if(!o.extract_temp && (directory.options & setup::directory_entry::DeleteAfterInstall)) {
			continue; // Ignore temporary dirs
		}
		
		if(!directory.languages.empty()) {
			if(!o.language.empty() && !setup::expression_match(o.language, directory.languages)) {
				continue; // Ignore other languages
			}
		} else if(o.language_only) {
			continue; // Ignore language-agnostic dirs
		}
		
		std::string path = o.filenames.convert(directory.name);
		if(path.empty()) {
			continue; // Don't know what to do with this
		}
		std::string internal_path = boost::algorithm::to_lower_copy(path);
		
		bool path_included = includes.match(internal_path);
		
		insert_dirs(processed.directories, includes, internal_path, path, path_included);
		
		DirectoriesMap::iterator it;
		if(path_included) {
			std::pair<DirectoriesMap::iterator, bool> existing = processed.directories.insert(
				std::make_pair(internal_path, processed_directory(path))
			);
			it = existing.first;
		} else {
			it = processed.directories.find(internal_path);
			if(it == processed.directories.end()) {
				continue;
			}
		}
		
		it->second.set_entry(&directory);
	}
	
	// Filter the files to be extracted
	BOOST_FOREACH(const setup::file_entry & file, info.files) {
		
		if(file.location >= info.data_entries.size()) {
			continue; // Ignore external files (copy commands)
		}
		
		if(!o.extract_temp && (file.options & setup::file_entry::DeleteAfterInstall)) {
			continue; // Ignore temporary files
		}
		
		if(!file.languages.empty()) {
			if(!o.language.empty() && !setup::expression_match(o.language, file.languages)) {
				continue; // Ignore other languages
			}
		} else if(o.language_only) {
			continue; // Ignore language-agnostic files
		}
		
		std::string path = o.filenames.convert(file.destination);
		if(path.empty()) {
			continue; // Internal file, not extracted
		}
		std::string internal_path = boost::algorithm::to_lower_copy(path);
		
		bool path_included = includes.match(internal_path);
		
		insert_dirs(processed.directories, includes, internal_path, path, path_included);
		
		if(!path_included) {
			continue; // Ignore excluded file
		}
		
		std::pair<FilesMap::iterator, bool> insertion = processed.files.insert(std::make_pair(
			internal_path, processed_file(&file, path)
		));
		
		if(!insertion.second) {
			// Collision!
			processed_file & existing = insertion.first->second;
			
			if(o.collisions == ErrorOnCollisions) {
				throw std::runtime_error("Collision: " + path);
			} else if(o.collisions == RenameAllCollisions) {
				collisions[internal_path].push_back(processed_file(&file, path));
			} else {
				
				const setup::data_entry & newdata = info.data_entries[file.location];
				const setup::data_entry & olddata = info.data_entries[existing.entry().location];
				const char * skip = handle_collision(existing.entry(), olddata, file, newdata);
				
				if(!o.default_language.empty()) {
					bool oldlang = setup::expression_match(o.default_language, file.languages);
					bool newlang = setup::expression_match(o.default_language, existing.entry().languages);
					if(oldlang && !newlang) {
						skip = NULL;
					} else if(!oldlang && newlang) {
						skip = "overwritten";
					}
				}
				
				if(o.collisions == RenameCollisions) {
					const setup::file_entry & clobberedfile = skip ? file : existing.entry();
					const std::string & clobberedpath = skip ? path : existing.path();
					collisions[internal_path].push_back(processed_file(&clobberedfile, clobberedpath));
				} else if(!o.silent) {
					std::cout << " - ";
					const std::string & clobberedpath = skip ? path : existing.path();
					std::cout << '"' << color::dim_yellow << clobberedpath << color::reset << '"';
					print_filter_info(skip ? file : existing.entry());
					if(o.list_sizes) {
						print_size_info(skip ? newdata.file : olddata.file, skip ? file.size : existing.entry().size);
					}
					if(o.list_checksums) {
						std::cout << ' ';
						print_checksum_info(skip ? newdata.file : olddata.file,
						                    skip ? &file.checksum : &existing.entry().checksum);
					}
					std::cout << " - " << (skip ? skip : "overwritten") << '\n';
				}
				
				if(!skip) {
					existing.set_entry(&file);
					if(file.type != setup::file_entry::UninstExe) {
						// Old file is "deleted" first → use case from new file
						existing.set_path(path);
					}
				}
				
			}
			
		}
		
	}
	
	if(o.collisions == RenameCollisions || o.collisions == RenameAllCollisions) {
		rename_collisions(o, processed.files, collisions);
	}
	
	return processed;
}

void create_output_directory(const extract_options & o) {
	
	try {
		if(!o.output_dir.empty() && !fs::exists(o.output_dir)) {
			fs::create_directory(o.output_dir);
		}
	} catch(...) {
		throw std::runtime_error("Could not create output directory \"" + o.output_dir.string() + '"');
	}
	
}

} // anonymous namespace

void process_file(const fs::path & installer, const extract_options & o) {
	
	bool is_directory;
	try {
		is_directory = fs::is_directory(installer);
	} catch(...) {
		throw std::runtime_error("Could not open file \"" + installer.string()
		                         + "\": access denied");
	}
	if(is_directory) {
		throw std::runtime_error("Input file \"" + installer.string() + "\" is a directory!");
	}
	
	util::ifstream ifs;
	try {
		ifs.open(installer, std::ios_base::in | std::ios_base::binary);
		if(!ifs.is_open()) {
			throw std::exception();
		}
	} catch(...) {
		throw std::runtime_error("Could not open file \"" + installer.string() + '"');
	}
	
	loader::offsets offsets;
	offsets.load(ifs);
	
	#ifdef DEBUG
	if(logger::debug) {
		print_offsets(offsets);
		std::cout << '\n';
	}
	#endif
	
	if(o.data_version)  {
		setup::version version;
		ifs.seekg(offsets.header_offset);
		version.load(ifs);
		if(o.silent) {
			std::cout << version << '\n';
		} else {
			std::cout << color::white << version << color::reset << '\n';
		}
		return;
	}
	
	#ifdef DEBUG
	if(o.dump_headers)  {
		create_output_directory(o);
		dump_headers(ifs, offsets, o);
		return;
	}
	#endif
	
	setup::info::entry_types entries = 0;
	if(o.list || o.test || o.extract || (o.gog_galaxy && o.list_languages)) {
		entries |= setup::info::Files;
		entries |= setup::info::Directories;
		entries |= setup::info::DataEntries;
	}
	if(o.list_languages) {
		entries |= setup::info::Languages;
	}
	if(o.gog_game_id || o.gog) {
		entries |= setup::info::RegistryEntries;
	}
	if(!o.extract_unknown) {
		entries |= setup::info::NoUnknownVersion;
	}
#ifdef DEBUG
	if(logger::debug) {
		entries = setup::info::entry_types::all();
	}
#endif
	
	ifs.seekg(offsets.header_offset);
	setup::info info;
	try {
		info.load(ifs, entries, o.codepage);
	} catch(const setup::version_error &) {
		fs::path headerfile = installer;
		headerfile.replace_extension(".0");
		if(offsets.header_offset == 0 && headerfile != installer && fs::exists(headerfile)) {
			log_info << "Opening \"" << color::cyan << headerfile.string() << color::reset << '"';
			process_file(headerfile, o);
			return;
		}
		if(offsets.found_magic) {
			if(offsets.header_offset == 0) {
				throw format_error("Could not determine location of setup headers!");
			} else {
				throw format_error("Could not determine setup data version!");
			}
		}
		throw;
	} catch(const std::exception & e) {
		std::ostringstream oss;
		oss << "Stream error while parsing setup headers!\n";
		oss << " ├─ detected setup version: " << info.version << '\n';
		oss << " └─ error reason: " << e.what();
		throw format_error(oss.str());
	}
	
	if(o.gog_galaxy && (o.list || o.test || o.extract || o.list_languages)) {
		gog::parse_galaxy_files(info, o.gog);
	}
	
	bool multiple_sections = print_file_info(o, info);
	
	std::string password;
	if(o.password.empty()) {
		if(!o.quiet && (o.list || o.test || o.extract) && (info.header.options & setup::header::EncryptionUsed)) {
			log_warning << "Setup contains encrypted files, use the --password option to extract them";
		}
	} else {
		util::from_utf8(o.password, password, info.codepage);
		if(info.header.options & setup::header::Password) {
			crypto::hasher checksum(info.header.password.type);
			checksum.update(info.header.password_salt.c_str(), info.header.password_salt.length());
			checksum.update(password.c_str(), password.length());
			if(checksum.finalize() != info.header.password) {
				if(o.check_password) {
					throw std::runtime_error("Incorrect password provided");
				}
				log_error << "Incorrect password provided";
				password.clear();
			}
		}
		#if !INNOEXTRACT_HAVE_ARC4
		if((o.extract || o.test) && (info.header.options & setup::header::EncryptionUsed)) {
			log_warning << "ARC4 decryption not supported in this build, skipping compressed chunks";
		}
		password.clear();
		#endif
	}
	
	if(!o.list && !o.test && !o.extract) {
		return;
	}
	
	if(!o.silent && multiple_sections) {
		std::cout << "Files:\n";
	}
	
	processed_entries processed = filter_entries(o, info);
	
	if(o.extract) {
		create_output_directory(o);
	}
	
	if(o.list || o.extract) {
		
		BOOST_FOREACH(const DirectoriesMap::value_type & i, processed.directories) {
			
			const std::string & path = i.second.path();
			
			if(o.list && !i.second.implied()) {
				
				if(!o.silent) {
					
					std::cout << " - ";
					std::cout << '"' << color::dim_white << path << setup::path_sep << color::reset << '"';
					if(i.second.has_entry()) {
						print_filter_info(i.second.entry());
					}
					std::cout << '\n';
					
				} else {
					std::cout << color::dim_white << path << setup::path_sep << color::reset << '\n';
				}
				
			}
			
			if(o.extract) {
				fs::path dir = o.output_dir / path;
				try {
					fs::create_directory(dir);
				} catch(...) {
					throw std::runtime_error("Could not create directory \"" + dir.string() + '"');
				}
			}
			
		}
		
	}
	
	typedef std::pair<const processed_file *, boost::uint64_t> output_location;
	std::vector< std::vector<output_location> > files_for_location;
	files_for_location.resize(info.data_entries.size());
	BOOST_FOREACH(const FilesMap::value_type & i, processed.files) {
		const processed_file & file = i.second;
		files_for_location[file.entry().location].push_back(output_location(&file, 0));
		if(o.test || o.extract) {
			boost::uint64_t offset = info.data_entries[file.entry().location].uncompressed_size;
			boost::uint32_t sort_slice = info.data_entries[file.entry().location].chunk.first_slice;
			boost::uint32_t sort_offset = info.data_entries[file.entry().location].chunk.sort_offset;
			BOOST_FOREACH(boost::uint32_t location, file.entry().additional_locations) {
				setup::data_entry & data = info.data_entries[location];
				files_for_location[location].push_back(output_location(&file, offset));
				offset += data.uncompressed_size;
				if(data.chunk.first_slice > sort_slice ||
				   (data.chunk.first_slice == sort_slice && data.chunk.sort_offset > sort_offset)) {
					sort_slice = data.chunk.first_slice;
					sort_offset = data.chunk.sort_offset;
				} else if(data.chunk.first_slice == sort_slice && data.chunk.sort_offset == data.chunk.offset) {
					data.chunk.sort_offset = ++sort_offset;
				} else {
					// Could not reorder chunk - no point in trying to reordder the remaining chunks
					sort_slice = boost::uint32_t(-1);
				}
			}
		}
	}
	
	boost::uint64_t total_size = 0;
	
	typedef std::map<stream::file, size_t> Files;
	typedef std::map<stream::chunk, Files> Chunks;
	Chunks chunks;
	for(size_t i = 0; i < info.data_entries.size(); i++) {
		if(!files_for_location[i].empty()) {
			setup::data_entry & location = info.data_entries[i];
			chunks[location.chunk][location.file] = i;
			total_size += location.uncompressed_size;
		}
	}
	
	boost::scoped_ptr<stream::slice_reader> slice_reader;
	if(o.extract || o.test) {
		if(offsets.data_offset) {
			slice_reader.reset(new stream::slice_reader(&ifs, offsets.data_offset));
		} else {
			fs::path dir = installer.parent_path();
			std::string basename = util::as_string(installer.stem());
			std::string basename2 = info.header.base_filename;
			// Prevent access to unexpected files
			std::replace(basename2.begin(), basename2.end(), '/', '_');
			std::replace(basename2.begin(), basename2.end(), '\\', '_');
			// Older Inno Setup versions used the basename stored in the headers, change our default accordingly
			if(info.version < INNO_VERSION(4, 1, 7) && !basename2.empty()) {
				std::swap(basename2, basename);
			}
			slice_reader.reset(new stream::slice_reader(dir, basename, basename2, info.header.slices_per_disk));
		}
	}
	
	progress extract_progress(total_size);
	
	typedef boost::ptr_map<const processed_file *, file_output> multi_part_outputs;
	multi_part_outputs multi_outputs;
	
	BOOST_FOREACH(const Chunks::value_type & chunk, chunks) {
		
		debug("[starting " << chunk.first.compression << " chunk @ slice " << chunk.first.first_slice
		      << " + " << print_hex(offsets.data_offset) << " + " << print_hex(chunk.first.offset)
		      << ']');
		
		stream::chunk_reader::pointer chunk_source;
		if((o.extract || o.test) && (chunk.first.encryption == stream::Plaintext || !password.empty())) {
			chunk_source = stream::chunk_reader::get(*slice_reader, chunk.first, password);
		}
		boost::uint64_t offset = 0;
		
		BOOST_FOREACH(const Files::value_type & location, chunk.second) {
			const stream::file & file = location.first;
			const std::vector<output_location> & output_locations = files_for_location[location.second];
			
			if(file.offset > offset) {
				debug("discarding " << print_bytes(file.offset - offset)
				      << " @ " << print_hex(offset));
				if(chunk_source.get()) {
					util::discard(*chunk_source, file.offset - offset);
				}
			}
			
			// Print filename and size
			if(o.list) {
				
				extract_progress.clear(DeferredClear);
				
				if(!o.silent) {
					
					bool named = false;
					boost::uint64_t size = 0;
					const crypto::checksum * checksum = NULL;
					BOOST_FOREACH(const output_location & output, output_locations) {
						if(output.second != 0) {
							continue;
						}
						if(output.first->entry().size != 0) {
							if(size != 0 && size != output.first->entry().size) {
								log_warning << "Mismatched output sizes";
							}
							size = output.first->entry().size;
						}
						if(output.first->entry().checksum.type != crypto::None) {
							if(checksum && *checksum != output.first->entry().checksum) {
								log_warning << "Mismatched output checksums";
							}
							checksum = &output.first->entry().checksum;
						}
						if(named) {
							std::cout << ", ";
						} else {
							std::cout << " - ";
							named = true;
						}
						if(chunk.first.encryption != stream::Plaintext) {
							if(password.empty()) {
								std::cout << '"' << color::dim_yellow << output.first->path() << color::reset << '"';
							} else {
								std::cout << '"' << color::yellow << output.first->path() << color::reset << '"';
							}
						} else {
							std::cout << '"' << color::white << output.first->path() << color::reset << '"';
						}
						print_filter_info(output.first->entry());
					}
					
					if(named) {
						if(o.list_sizes) {
							print_size_info(file, size);
						}
						if(o.list_checksums) {
							std::cout << ' ';
							print_checksum_info(file, checksum);
						}
						if(chunk.first.encryption != stream::Plaintext && password.empty()) {
							std::cout << " - encrypted";
						}
						std::cout << '\n';
					}
					
				} else {
					BOOST_FOREACH(const output_location & output, output_locations) {
						if(output.second == 0) {
							const processed_file * fileinfo = output.first;
							if(o.list_sizes) {
								boost::uint64_t size = fileinfo->entry().size;
								std::cout << color::dim_cyan << (size != 0 ? size : file.size) << color::reset << ' ';
							}
							if(o.list_checksums) {
								print_checksum_info(file, &fileinfo->entry().checksum);
								std::cout << ' ';
							}
							std::cout << color::white << fileinfo->path() << color::reset << '\n';
						}
					}
				}
				
				bool updated = extract_progress.update(0, true);
				if(!updated && (o.extract || o.test)) {
					std::cout.flush();
				}
				
			}
			
			// Seek to the correct position within the chunk
			if(chunk_source.get() && file.offset < offset) {
				std::ostringstream oss;
				oss << "Bad offset while extracting files: file start (" << file.offset
				    << ") is before end of previous file (" << offset << ")!";
				throw format_error(oss.str());
			}
			offset = file.offset + file.size;
			
			if(!chunk_source.get()) {
				continue; // Not extracting/testing this file
			}
			
			crypto::checksum checksum;
			
			// Open input file
			stream::file_reader::pointer file_source;
			file_source = stream::file_reader::get(*chunk_source, file, &checksum);
			
			// Open output files
			boost::ptr_vector<file_output> single_outputs;
			std::vector<file_output *> outputs;
			BOOST_FOREACH(const output_location & output_loc, output_locations) {
				const processed_file * fileinfo = output_loc.first;
				try {
					
					if(!o.extract && fileinfo->entry().checksum.type == crypto::None) {
						continue;
					}
					
					// Re-use existing file output for multi-part files
					file_output * output = NULL;
					if(fileinfo->is_multipart()) {
						multi_part_outputs::iterator it = multi_outputs.find(fileinfo);
						if(it != multi_outputs.end()) {
							output = it->second;
						}
					}
					
					if(!output) {
						output = new file_output(o.output_dir, fileinfo, o.extract);
						if(fileinfo->is_multipart()) {
							multi_outputs.insert(fileinfo, output);
						} else {
							single_outputs.push_back(output);
						}
					}
					
					outputs.push_back(output);
					
					output->seek(output_loc.second);
					
				} catch(boost::bad_pointer &) {
					// should never happen
					std::terminate();
				}
			}
			
			// Copy data
			boost::uint64_t output_size = 0;
			while(!file_source->eof()) {
				char buffer[8192 * 10];
				std::streamsize buffer_size = std::streamsize(boost::size(buffer));
				std::streamsize n = file_source->read(buffer, buffer_size).gcount();
				if(n > 0) {
					BOOST_FOREACH(file_output * output, outputs) {
						bool success = output->write(buffer, size_t(n));
						if(!success) {
							throw std::runtime_error("Error writing file \"" + output->path().string() + '"');
						}
					}
					extract_progress.update(boost::uint64_t(n));
					output_size += boost::uint64_t(n);
				}
			}
			
			const setup::data_entry & data = info.data_entries[location.second];
			
			if(output_size != data.uncompressed_size) {
				log_warning << "Unexpected output file size: " << output_size << " != " << data.uncompressed_size;
			}
			
			util::time filetime = data.timestamp;
			if(o.extract && o.preserve_file_times && o.local_timestamps && !(data.options & data.TimeStampInUTC)) {
				filetime = util::to_local_time(filetime);
			}
			
			BOOST_FOREACH(file_output * output, outputs) {
				
				if(output->file()->is_multipart() && !output->is_complete()) {
					continue;
				}
				
				// Verify output checksum if available
				if(output->file()->entry().checksum.type != crypto::None && output->calculate_checksum()) {
					crypto::checksum output_checksum = output->checksum();
					if(output_checksum != output->file()->entry().checksum) {
						log_warning << "Output checksum mismatch for " << output->file()->path() << ":\n"
						            << " ├─ actual:   " << output_checksum << '\n'
						            << " └─ expected: " << output->file()->entry().checksum;
						if(o.test) {
							throw std::runtime_error("Integrity test failed!");
						}
					}
				}
				
				// Adjust file timestamps
				if(o.extract && o.preserve_file_times) {
					output->close();
					if(!util::set_file_time(output->path(), filetime, data.timestamp_nsec)) {
						log_warning << "Error setting timestamp on file " << output->path();
					}
				}
				
				if(output->file()->is_multipart()) {
					debug("[finalizing multi-part file]");
					multi_outputs.erase(output->file());
				}
				
			}
			
			// Verify checksums
			if(checksum != file.checksum) {
				log_warning << "Checksum mismatch:\n"
				            << " ├─ actual:   " << checksum << '\n'
				            << " └─ expected: " << file.checksum;
				if(o.test) {
					throw std::runtime_error("Integrity test failed!");
				}
			}
			
		}
		
		#ifdef DEBUG
		if(offset < chunk.first.size) {
			debug("discarding " << print_bytes(chunk.first.size - offset)
			      << " at end of chunk @ " << print_hex(offset));
		}
		#endif
	}
	
	extract_progress.clear();
	
	if(!multi_outputs.empty()) {
		log_warning << "Incomplete multi-part files";
	}
	
	if(o.warn_unused || o.gog) {
		gog::probe_bin_files(o, info, installer, offsets.data_offset == 0);
	}
	
}

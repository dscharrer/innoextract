/*
 * Copyright (C) 2014-2019 Daniel Scharrer
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

#include "cli/gog.hpp"

#include <stddef.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <signal.h>

#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>
#include <boost/noncopyable.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>

#include "cli/extract.hpp"

#include "crypto/md5.hpp"

#include "loader/offsets.hpp"

#include "setup/data.hpp"
#include "setup/info.hpp"
#include "setup/registry.hpp"

#include "stream/slice.hpp"

#include "util/boostfs_compat.hpp"
#include "util/console.hpp"
#include "util/encoding.hpp"
#include "util/fstream.hpp"
#include "util/log.hpp"
#include "util/process.hpp"

namespace fs = boost::filesystem;

namespace gog {

std::string get_game_id(const setup::info & info) {
	
	std::string id;
	
	const char * prefix = "SOFTWARE\\GOG.com\\Games\\";
	size_t prefix_length = std::strlen(prefix);
	
	BOOST_FOREACH(const setup::registry_entry & entry, info.registry_entries) {
		
		if(!boost::istarts_with(entry.key, prefix)) {
			continue;
		}
		
		if(entry.key.find('\\', prefix_length) != std::string::npos) {
			continue;
		}
		
		if(boost::iequals(entry.name, "gameID")) {
			id = entry.value;
			util::to_utf8(id, info.codepage);
			break;
		}
		
		if(id.empty()) {
			id = entry.key.substr(prefix_length);
		}
		
	}
	
	return id;
}

namespace {

std::string get_verb(const extract_options & o) {
	const char * verb = "inspect";
	if(o.extract) {
		verb = "extract";
	} else if(o.test) {
		verb = "test";
	} else if(o.list) {
		verb = "list the contents of";
	}
	return verb;
}

volatile sig_atomic_t quit_requested = 0;
void quit_handler(int /* ignored */) {
	quit_requested = 1;
}

bool process_file_unrar(const std::string & file, const extract_options & o, const std::string & password) {
	
	std::vector<const char *> args;
	args.push_back("unrar");
	
	if(o.extract) {
		args.push_back("x");
	} else if(o.test) {
		args.push_back("t");
	} else if(o.silent) {
		args.push_back("lb");
	} else {
		args.push_back("l");
	}
	
	args.push_back("-p-");
	std::string pwarg;
	if(!password.empty()) {
		pwarg = "-p" + password;
		args.push_back(pwarg.c_str());
	}
	
	args.push_back("-idc"); // Disable copyright header
	
	if(!progress::is_enabled()) {
		args.push_back("-idp"); // Disable progress display
	}
	
	if(o.filenames.is_lowercase()) {
		args.push_back("-cl"); // Connvert filenames to lowercase
	}
	
	if(!o.list) {
		args.push_back("-idq"); // Disable file list
	}
	
	args.push_back("-o+"); // Overwrite existing files
	
	if(o.preserve_file_times) {
		args.push_back("-tsmca"); // Restore file times
	} else {
		args.push_back("-tsm0c0a0"); // Don't restore file times
	}
	
	args.push_back("-y"); // Enable batch mode
	
	args.push_back("--");
	
	args.push_back(file.c_str());
	
	std::string dir = o.output_dir.string();
	if(!dir.empty()) {
		if(dir[dir.length() - 1] != '/' && dir[dir.length() - 1] != '\\') {
			#if defined(_WIN32)
			dir += '\\';
			#else
			dir += '/';
			#endif
		}
		args.push_back(dir.c_str());
	}
	
	args.push_back(NULL);
	
	int ret = util::run(&args.front());
	if(ret < 0 && !quit_requested) {
		args[0] = "rar";
		ret = util::run(&args.front());
		if(ret < 0 && !quit_requested) {
			return false;
		}
	}
	
	if(ret > 0) {
		throw std::runtime_error("Could not " + get_verb(o) + " \"" + file + "\": unrar failed");
	}
	
	return true;
}

bool process_file_unar(const std::string & file, const extract_options & o, const std::string & password) {
	
	std::string dir = o.output_dir.string();
	
	std::vector<const char *> args;
	if(o.extract) {
		args.push_back("unar");
		
		args.push_back("-f"); // Overwrite existing files
		
		args.push_back("-D"); // Don't create directory
		
		if(!dir.empty()) {
			args.push_back("-o");
			args.push_back(dir.c_str());
		}
		
		if(!o.list) {
			args.push_back("-q"); // Disable file list
		}
		
	} else {
		args.push_back("lsar");
		
		if(o.test) {
			args.push_back("-t");
		}
	}
	
	if(!password.empty()) {
		args.push_back("-p");
		args.push_back(password.c_str());
	}
	
	args.push_back("--");
	
	args.push_back(file.c_str());
	
	args.push_back(NULL);
	
	int ret = util::run(&args.front());
	if(ret < 0 && !quit_requested) {
		return false;
	}
	
	if(ret > 0) {
		throw std::runtime_error("Could not " + get_verb(o) + " \"" + file + "\": unar failed");
	}
	
	return true;
}

bool process_rar_file(const std::string & file, const extract_options & o, const std::string & password) {
	return process_file_unrar(file, o, password) || process_file_unar(file, o, password);
}

char hex_char(int c) {
	if(c < 10) {
		return char('0' + c);
	} else {
		return char('a' + (c - 10));
	}
}

class temporary_directory : private boost::noncopyable {
	
	fs::path parent;
	fs::path path;
	
public:
	
	explicit temporary_directory(const fs::path & base) {
		try {
			if(!base.empty() && !fs::exists(base)) {
				fs::create_directory(base);
				parent = base;
			}
			size_t tmpnum = 0;
			std::ostringstream oss;
			do {
				oss.str(std::string());
				oss << "innoextract-tmp-" << tmpnum++;
				path = base / oss.str();
			} while(fs::exists(path));
			fs::create_directory(path);
		} catch(...) {
			path = fs::path();
			throw std::runtime_error("Could not create temporary directory!");
		}
	}
	
	~temporary_directory() {
		if(!path.empty()) {
			try {
				fs::remove_all(path);
				if(!parent.empty()) {
					fs::remove(parent);
				}
			} catch(...) {
				log_error << "Could not remove temporary directory " << path << '!';
			}
		}
	}
	
	const fs::path & get() { return path; }
	
};

void process_rar_files(const std::vector<fs::path> & files,
                       const extract_options & o, const setup::info & info) {
	
	if((!o.list && !o.test && !o.extract) || files.empty()) {
		return;
	}
	
	// Calculate password from the GOG.com game ID
	std::string password = get_game_id(info);
	if(!password.empty()) {
		crypto::md5 md5;
		md5.init();
		md5.update(password.c_str(), password.length());
		char hash[16];
		md5.finalize(hash);
		password.resize(size_t(boost::size(hash) * 2));
		for(size_t i = 0; i < size_t(boost::size(hash)); i++) {
			password[2 * i + 0] = hex_char(boost::uint8_t(hash[i]) / 16);
			password[2 * i + 1] = hex_char(boost::uint8_t(hash[i]) % 16);
		}
	}
	
	if((!o.extract && !o.test && o.list) || files.size() == 1) {
		
		// When listing contents or for single-file archives, pass the bin file to unrar
		
		bool ok = true;
		BOOST_FOREACH(const fs::path & file, files) {
			if(!process_rar_file(file.string(), o, password)) {
				ok = false;
			}
		}
		
		if(ok) {
			return;
		}
		
	} else {
		
		/*
		 * When extracting multi-part archives we need to create symlinks with special
		 * names so that unrar will find all the parts of the archive.
		 */
		
		typedef void(*signal_handler /* … */)(int /* … */);
		#ifdef SIGINT
		signal_handler old_sigint_handler = signal(SIGINT, quit_handler);
		#endif
		#ifdef SIGTERM
		signal_handler old_sigterm_handler = signal(SIGTERM, quit_handler);
		#endif
		#ifdef SIGHUP
		signal_handler old_sighup_handler = signal(SIGHUP, quit_handler);
		#endif
		
		temporary_directory tmpdir(o.output_dir);
		
		fs::path first_file;
		try {
			
			fs::path here = fs::current_path();
			
			std::string basename = util::as_string(files.front().stem());
			if(boost::ends_with(basename, "-1")) {
				basename.resize(basename.length() - 2);
			}
			
			size_t i = 0;
			std::ostringstream oss;
			BOOST_FOREACH(const fs::path & file, files) {
				
				oss.str(std::string());
				oss << basename << ".r" << std::setfill('0') << std::setw(2) << i;
				fs::path symlink = tmpdir.get() / oss.str();
				
				if(file.root_path().empty()) {
					fs::create_symlink(here / file, symlink);
				} else {
					fs::create_symlink(file, symlink);
				}
				
				if(i == 0) {
					first_file = symlink;
				}
				
				i++;
			}
			
		} catch(...) {
			throw std::runtime_error("Could not " + get_verb(o)
			                         + " \"" + files.front().string()
			                         + "\": unable to create .r?? symlinks");
		}
		
		if(process_rar_file(first_file.string(), o, password)) {
			return;
		}
		
		#ifdef SIGHUP
		signal(SIGHUP, old_sighup_handler);
		#endif
		#ifdef SIGTERM
		signal(SIGTERM, old_sigterm_handler);
		#endif
		#ifdef SIGINT
		signal(SIGINT, old_sigint_handler);
		#endif
		if(quit_requested) {
			throw std::runtime_error("Aborted!");
		}
		
	}
	
	throw std::runtime_error("Could not " + get_verb(o) + " \"" + files.front().string()
	                         + "\": install `unrar` or `unar`");
}

void process_bin_files(const std::vector<fs::path> & files, const extract_options & o,
                      const setup::info & info) {
	
	util::ifstream ifs(files.front(), std::ios_base::in | std::ios_base::binary);
	if(!ifs.is_open()) {
		throw std::runtime_error("Could not open file \"" + files.front().string() + '"');
	}
	
	char magic[4];
	if(!ifs.read(magic, std::streamsize(boost::size(magic))).fail()) {
		
		if(std::memcmp(magic, "Rar!", 4) == 0) {
			ifs.close();
			process_rar_files(files, o, info);
			return;
		}
		
		if(std::memcmp(magic, "MZ", 2) == 0) {
			loader::offsets offsets;
			offsets.load(ifs);
			if(offsets.header_offset != 0) {
				ifs.close();
				extract_options new_options = o;
				new_options.gog = false;
				new_options.warn_unused = false;
				std::cout << '\n';
				process_file(files.front(), new_options);
				return;
			}
		}
		
	}
	
	throw std::runtime_error("Could not " + get_verb(o) + " \"" + files.front().string()
	                         + "\": unknown filetype");
}

size_t probe_bin_file_series(const extract_options & o, const setup::info & info, const fs::path & dir,
                             const std::string & basename, size_t format = 0, size_t start = 0) {
	
	size_t count = 0;
	
	std::vector<fs::path> files;
	
	for(size_t i = start;; i++) {
		
		fs::path file;
		if(format == 0) {
			file = dir / basename;
		} else {
			file = dir / stream::slice_reader::slice_filename(basename, i, format);
		}
		
		try {
			if(!fs::is_regular_file(file)) {
				break;
			}
		} catch(...) {
			break;
		}
		
		if(o.gog) {
			files.push_back(file);
		} else {
			log_warning << file.filename() << " is not part of the installer!";
			count++;
		}
		
		if(format == 0) {
			break;
		}
		
	}
	
	if(!files.empty()) {
		process_bin_files(files, o, info);
	}
	
	return count;
}

} // anonymous namespace

void probe_bin_files(const extract_options & o, const setup::info & info,
                     const fs::path & setup_file, bool external) {
	
	boost::filesystem::path dir = setup_file.parent_path();
	std::string basename = util::as_string(setup_file.stem());
	
	size_t bin_count = 0;
	bin_count += probe_bin_file_series(o, info, dir, basename + ".bin");
	bin_count += probe_bin_file_series(o, info, dir, basename + "-0" + ".bin");
	

	boost::uint32_t max_slice = 0;
	if(external) {
		BOOST_FOREACH(const setup::data_entry & location, info.data_entries) {
			max_slice = std::max(max_slice, location.chunk.first_slice);
			max_slice = std::max(max_slice, location.chunk.last_slice);
		}
	}
	
	size_t slice =  0;
	size_t format = 1;
	if(external && info.header.slices_per_disk == 1) {
		slice = size_t(max_slice) + 1;
	}
	bin_count += probe_bin_file_series(o, info, dir, basename, format, slice);
	
	slice = 0;
	format = 2;
	if(external && info.header.slices_per_disk != 1) {
		slice = size_t(max_slice) + 1;
		format = info.header.slices_per_disk;
	}
	bin_count += probe_bin_file_series(o, info, dir, basename, format, slice);
	
	if(bin_count) {
		const char * verb = "inspecting";
		if(o.extract) {
			verb = "extracting";
		} else if(o.test) {
			verb = "testing";
		} else if(o.list) {
			verb = "listing the contents of";
		}
		std::cerr << color::yellow << "Use the --gog option to try " << verb << " "
		          << (bin_count > 1 ? "these files" : "this file") << ".\n" << color::reset;
	}
	
}

} // namespace gog

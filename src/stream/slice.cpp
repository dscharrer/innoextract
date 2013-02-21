/*
 * Copyright (C) 2011-2012 Daniel Scharrer
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

#include "stream/slice.hpp"

#include <sstream>
#include <cstring>
#include <limits>

#include <boost/cstdint.hpp>

#include "util/console.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/util.hpp"

namespace stream {

namespace {

const char slice_ids[][8] = {
	{ 'i', 'd', 's', 'k', 'a', '1', '6', 0x1a },
	{ 'i', 'd', 's', 'k', 'a', '3', '2', 0x1a },
};

} // anonymous namespace

slice_reader::slice_reader(std::istream * istream, boost::uint32_t data_offset)
	: data_offset(data_offset),
	  dir(), last_dir(), base_file(), slices_per_disk(1),
	  current_slice(0), slice_file(), slice_size(0),
	  ifs(), is(istream) {
	
	std::streampos max_size = std::streampos(std::numeric_limits<boost::int32_t>::max());
	
	std::streampos file_size = is->seekg(0, std::ios_base::end).tellg();
	
	slice_size = boost::uint32_t(std::min(file_size, max_size));
	if(is->seekg(data_offset).fail()) {
		throw slice_error("could not seek to data");
	}
}

slice_reader::slice_reader(const path_type & dir, const path_type & base_file,
                           size_t slices_per_disk)
	: data_offset(0),
	  dir(dir), last_dir(dir), base_file(base_file), slices_per_disk(slices_per_disk),
	  current_slice(0), slice_file(), slice_size(0),
	  ifs(), is(&ifs) { }

bool slice_reader::seek(size_t slice) {
	
	if(slice == current_slice && is_open()) {
		return true;
	}
	
	if(data_offset != 0) {
		throw slice_error("[slice] cannot change slices in single-file setup");
	}
	
	return open(slice, path_type());
}

bool slice_reader::open_file(const path_type & file) {
	
	log_info << "opening \"" << color::cyan << file.string() << color::reset << '"';
	
	ifs.close();
	ifs.clear();
	
	ifs.open(file, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	if(ifs.fail()) {
		return false;
	}
	
	std::streampos file_size = ifs.tellg();
	ifs.seekg(0);
	
	char magic[8];
	if(ifs.read(magic, 8).fail()) {
		ifs.close();
		throw slice_error("error reading slice magic number");
	}
	bool found = false;
	for(size_t i = 0; ARRAY_SIZE(slice_ids); i++) {
		if(!std::memcmp(magic, slice_ids[i], 8)) {
			found = true;
			break;
		}
	}
	if(!found) {
		ifs.close();
		throw slice_error("bad slice magic number");
	}
	
	slice_size = load_number<boost::uint32_t>(ifs);
	if(ifs.fail()) {
		ifs.close();
		throw slice_error("error reading slice size");
	} else if(std::streampos(slice_size) > file_size) {
		ifs.close();
		std::ostringstream oss;
		oss << "bad slice size: " << slice_size << " > " << file_size;
		throw slice_error(oss.str());
	} else if(std::streampos(slice_size) < ifs.tellg()) {
		ifs.close();
		std::ostringstream oss;
		oss << "bad slice size: " << slice_size << " < " << ifs.tellg();
		throw slice_error(oss.str());
	}
	
	slice_file = file;
	
	last_dir = file.parent_path();
	
	return true;
}

bool slice_reader::open(size_t slice, const path_type & file) {
	
	current_slice = slice;
	is = &ifs;
	ifs.close();
	
	if(slices_per_disk == 0) {
		throw std::runtime_error("[slice] slices per disk must not be zero");
	}
	
	path_type slice_file = file;
	
	if(slice_file.empty()) {
		
		std::ostringstream oss;
		oss << base_file.string() << '-';
		if(slices_per_disk == 1) {
			oss << (slice + 1);
		} else {
			size_t major = (slice / slices_per_disk) + 1;
			size_t minor = slice % slices_per_disk;
			oss << major << char(boost::uint8_t('a') + minor);
		}
		oss << ".bin";
		
		slice_file = oss.str();
	}
	
	if(open_file(last_dir / slice_file)) {
		return true;
	}
	
	if(dir != last_dir && open_file(dir / slice_file)) {
		return true;
	}
	
	if(dir != last_dir) {
		log_error << "error opening " << slice_file << " in " << last_dir << " or " << dir;
	} else {
		log_error << "error opening " << last_dir / slice_file;
	}
	
	return false;
}

bool slice_reader::seek(size_t slice, boost::uint32_t offset) {
	
	if(!seek(slice)) {
		return false;
	}
	
	offset += data_offset;
	
	if(offset > slice_size) {
		return false;
	}
	
	if(is->seekg(offset).fail()) {
		return false;
	}
	
	return true;
}

std::streamsize slice_reader::read(char * buffer, std::streamsize bytes) {
	
	if(!seek(current_slice)) {
		return (bytes == 0) ? 0 : -1;
	}
	
	std::streamsize nread = 0;
	
	while(bytes > 0) {
		
		boost::uint32_t read_pos = boost::uint32_t(is->tellg());
		if(read_pos > slice_size) {
			break;
		}
		std::streamsize remaining = std::streamsize(slice_size - read_pos);
		if(!remaining) {
			if(!seek(current_slice + 1)) {
				break;
			}
			read_pos = boost::uint32_t(is->tellg());
			if(read_pos > slice_size) {
				break;
			}
			remaining = std::streamsize(slice_size - read_pos);
		}
		
		if(is->read(buffer, std::min(remaining, bytes)).fail()) {
			break;
		}
		
		std::streamsize read = is->gcount();
		nread += read, buffer += read, bytes -= read;
	}
	
	return (nread != 0 || bytes == 0) ? nread : -1;
}

} // namespace stream

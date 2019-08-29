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

#include "stream/slice.hpp"

#include <sstream>
#include <cstring>
#include <limits>

#include <boost/cstdint.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/range/size.hpp>

#include "util/console.hpp"
#include "util/load.hpp"
#include "util/log.hpp"

namespace stream {

namespace {

const char slice_ids[][8] = {
	{ 'i', 'd', 's', 'k', 'a', '1', '6', 0x1a },
	{ 'i', 'd', 's', 'k', 'a', '3', '2', 0x1a },
};

} // anonymous namespace

slice_reader::slice_reader(std::istream * istream, boost::uint32_t offset)
	: data_offset(offset),
	  slices_per_disk(1), current_slice(0), slice_size(0),
	  is(istream) {
	
	std::streampos max_size = std::streampos(std::numeric_limits<boost::int32_t>::max());
	
	std::streampos file_size = is->seekg(0, std::ios_base::end).tellg();
	
	slice_size = boost::uint32_t(std::min(file_size, max_size));
	if(is->seekg(data_offset).fail()) {
		throw slice_error("could not seek to data");
	}
}

slice_reader::slice_reader(const path_type & dirname, const std::string & basename,
                           const std::string & basename2, size_t disk_slice_count)
	: data_offset(0),
	  dir(dirname), base_file(basename), base_file2(basename2),
	  slices_per_disk(disk_slice_count), current_slice(0), slice_size(0),
	  is(&ifs) { }

void slice_reader::seek(size_t slice) {
	
	if(slice == current_slice && is_open()) {
		return;
	}
	
	if(data_offset != 0) {
		throw slice_error("cannot change slices in single-file setup");
	}
	
	open(slice);
}

bool slice_reader::open_file(const path_type & file) {
	
	if(!boost::filesystem::exists(file)) {
		return false;
	}
	
	log_info << "Opening \"" << color::cyan << file.string() << color::reset << '"';
	
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
		throw slice_error("could not read slice magic number in \"" + file.string() + "\"");
	}
	bool found = false;
	for(size_t i = 0; boost::size(slice_ids); i++) {
		if(!std::memcmp(magic, slice_ids[i], 8)) {
			found = true;
			break;
		}
	}
	if(!found) {
		ifs.close();
		throw slice_error("bad slice magic number in \"" + file.string() + "\"");
	}
	
	slice_size = util::load<boost::uint32_t>(ifs);
	if(ifs.fail()) {
		ifs.close();
		throw slice_error("could not read slice size in \"" + file.string() + "\"");
	} else if(std::streampos(slice_size) > file_size) {
		ifs.close();
		std::ostringstream oss;
		oss << "bad slice size in " << file << ": " << slice_size << " > " << file_size;
		throw slice_error(oss.str());
	} else if(std::streampos(slice_size) < ifs.tellg()) {
		ifs.close();
		std::ostringstream oss;
		oss << "bad slice size in " << file << ": " << slice_size << " < " << ifs.tellg();
		throw slice_error(oss.str());
	}
	
	return true;
}

std::string slice_reader::slice_filename(const std::string & basename, size_t slice,
                                         size_t slices_per_disk) {
	
	std::ostringstream oss;
	oss << basename << '-';
	
	if(slices_per_disk == 0) {
		throw slice_error("slices per disk must not be zero");
	}
	
	if(slices_per_disk == 1) {
		oss << (slice + 1);
	} else {
		size_t major = (slice / slices_per_disk) + 1;
		size_t minor = slice % slices_per_disk;
		oss << major << char(boost::uint8_t('a') + minor);
	}
	
	oss << ".bin";
	
	return oss.str();
}

bool slice_reader::open_file_case_insensitive(const path_type & dirname, const path_type & filename) {
	
	boost::filesystem::directory_iterator end;
	for(boost::filesystem::directory_iterator i(dirname); i != end; ++i) {
		path_type actual_filename = i->path().filename();
		if(boost::iequals(actual_filename.string(), filename.string()) && open_file(dirname / actual_filename)) {
			return true;
		}
	}
	
	return false;
}

void slice_reader::open(size_t slice) {
	
	current_slice = slice;
	is = &ifs;
	ifs.close();
	
	path_type slice_file = slice_filename(base_file, slice, slices_per_disk);
	if(open_file(dir / slice_file)) {
		return;
	}
	
	path_type slice_file2 = slice_filename(base_file2, slice, slices_per_disk);
	if(!base_file2.empty() && slice_file2 != slice_file && open_file(dir / slice_file2)) {
		return;
	}
	
	if(open_file_case_insensitive(dir, slice_file)) {
		return;
	}
	
	if(!base_file2.empty() && slice_file2 != slice_file && open_file_case_insensitive(dir, slice_file2)) {
		return;
	}
	
	std::ostringstream oss;
	oss << "could not open slice " << slice << ": " << slice_file;
	if(!base_file2.empty() && slice_file2 != slice_file) {
		oss << " or " << slice_file2;
	}
	throw slice_error(oss.str());
}

bool slice_reader::seek(size_t slice, boost::uint32_t offset) {
	
	seek(slice);
	
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
	
	seek(current_slice);
	
	std::streamsize nread = 0;
	
	while(bytes > 0) {
		
		boost::uint32_t read_pos = boost::uint32_t(is->tellg());
		if(read_pos > slice_size) {
			break;
		}
		boost::uint32_t remaining = slice_size - read_pos;
		if(!remaining) {
			seek(current_slice + 1);
			read_pos = boost::uint32_t(is->tellg());
			if(read_pos > slice_size) {
				break;
			}
			remaining = slice_size - read_pos;
		}
		
		boost::uint64_t toread = std::min(boost::uint64_t(remaining), boost::uint64_t(bytes));
		toread = std::min(toread, boost::uint64_t(std::numeric_limits<std::streamsize>::max()));
		if(is->read(buffer, std::streamsize(toread)).fail()) {
			break;
		}
		
		std::streamsize read = is->gcount();
		nread += read, buffer += read, bytes -= read;
	}
	
	return (nread != 0 || bytes == 0) ? nread : -1;
}

} // namespace stream

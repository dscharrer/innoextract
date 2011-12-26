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

#include "stream/slice.hpp"

#include <stdint.h>
#include <sstream>
#include <cstring>
#include <limits>

#include "util/console.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/util.hpp"

using std::string;

namespace stream {

namespace {

const char slice_ids[][8] = {
	{ 'i', 'd', 's', 'k', 'a', '1', '6', 0x1a },
	{ 'i', 'd', 's', 'k', 'a', '3', '2', 0x1a },
};

} // anonymous namespace

slice_reader::slice_reader(const string & setup_file, uint32_t data_offset)
	: dir(), last_dir(), base_file(), data_offset(data_offset), slices_per_disk(1),
	  current_slice(0) {
	
	ifs.open(setup_file.c_str(), std::ios_base::binary | std::ios_base::in | std::ios_base::ate);
	
	slice_size = uint32_t(std::min<std::streampos>(ifs.tellg(), std::numeric_limits<int32_t>::max()));
	if(ifs.seekg(data_offset).fail()) {
		ifs.close();
	}
}

slice_reader::slice_reader(const std::string & dir, const std::string & base_file,
                           size_t slices_per_disk)
	: dir(dir), last_dir(dir), base_file(base_file), data_offset(0), slices_per_disk(slices_per_disk),
	  current_slice(0) { }

bool slice_reader::seek(size_t slice) {
	
	if(slice == current_slice && ifs.is_open()) {
		return true;
	}
	
	if(data_offset != 0) {
		log_error << "[slice] cannot change slices in single-file setup";
		return false;
	}
	
	return open(slice, string());
}

bool slice_reader::openFile(const std::string & file) {
	
	log_info << "[slice] opening " << file;
	
	ifs.close();
	
	ifs.open(file.c_str(), std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	if(ifs.fail()) {
		return false;
	}
	
	std::streampos fileSize = ifs.tellg();
	ifs.seekg(0);
	
	char magic[8];
	if(ifs.read(magic, 8).fail()) {
		ifs.close();
		log_error << "[slice] error reading magic number";
		return false;
	}
	bool found = false;
	for(size_t i = 0; ARRAY_SIZE(slice_ids); i++) {
		if(!std::memcmp(magic, slice_ids[i], 8)) {
			found = true;
			break;
		}
	}
	if(!found) {
		log_error << "[slice] bad magic number";
		ifs.close();
		return false;
	}
	
	slice_size = load_number<uint32_t>(ifs);
	if(ifs.fail() || std::streampos(slice_size) > fileSize) {
		log_error << "[slice] bad slice size: " << slice_size << " > " << fileSize;
		ifs.close();
		return false;
	}
	
	slice_file = file;
	
	size_t dirpos = file.find_last_of('/');
	if(dirpos == string::npos) {
		last_dir.clear();
	} else {
		last_dir = file.substr(0, dirpos + 1);
	}
	
	return true;
}

bool slice_reader::open(size_t slice, const std::string & file) {
	
	current_slice = slice;
	ifs.close();
	
	if(slices_per_disk == 0) {
		log_error << "[slice] slices per disk must not be zero";
		return false;
	}
	
	std::string sliceFile = file;
	
	if(sliceFile.empty()) {
		
		std::ostringstream oss;
		oss << base_file << '-';
		if(slices_per_disk == 1) {
			oss << (slice + 1);
		} else {
			size_t major = (slice / slices_per_disk) + 1;
			size_t minor = slice % slices_per_disk;
			oss << major << char(uint8_t('a') + minor);
		}
		oss << ".bin";
		
		sliceFile = oss.str();
	}
	
	if(sliceFile[0] == '/') {
		return openFile(sliceFile);
	}
	
	if(openFile(last_dir + sliceFile)) {
		return true;
	}
	
	if(dir != last_dir && openFile(dir + sliceFile)) {
		return true;
	}
	
	return false;
}

bool slice_reader::seek(size_t slice, uint32_t offset) {
	
	if(!seek(slice)) {
		return false;
	}
	
	if(offset > slice_size - data_offset) {
		return false;
	}
	
	if(ifs.seekg(data_offset + offset).fail()) {
		return false;
	}
	
	return true;
}

std::streamsize slice_reader::read(char * buffer, std::streamsize bytes) {
	
	std::streamsize nread = 0;
	
	if(!seek(current_slice)) {
		return nread;
	}
	
	while(bytes > 0) {
		
		std::streamsize remaining = std::streamsize(slice_size - size_t(ifs.tellg()));
		if(!remaining) {
			if(!seek(current_slice + 1)) {
				return nread;
			}
			remaining = std::streamsize(slice_size - size_t(ifs.tellg()));
		}
		
		std::streamsize read = ifs.read(buffer, std::min(remaining, bytes)).gcount();
		
		nread += read, buffer += read, bytes -= read;
	}
	
	return nread;
}

} // namespace stream

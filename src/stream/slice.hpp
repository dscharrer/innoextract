/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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

#ifndef INNOEXTRACT_STREAM_SLICEREADER_HPP
#define INNOEXTRACT_STREAM_SLICEREADER_HPP

#include <boost/iostreams/concepts.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

namespace stream {

class slice_reader : public boost::iostreams::source {
	
	typedef boost::filesystem::path path_type;
	
	path_type dir;
	path_type last_dir;
	path_type base_file;
	const uint32_t data_offset;
	const size_t slices_per_disk;
	
	size_t current_slice;
	path_type slice_file;
	uint32_t slice_size;
	
	boost::filesystem::ifstream ifs;
	
	bool seek(size_t slice);
	bool open_file(const path_type & file);
	
public:
	
	slice_reader(const path_type & setup_file, uint32_t data_offset);
	
	slice_reader(const path_type & dir, const path_type & base_file, size_t slices_per_disk);
	
	bool seek(size_t slice, uint32_t offset);
	
	std::streamsize read(char * buffer, std::streamsize bytes);
	
	size_t slice() { return current_slice; }
	path_type & file() { return slice_file; }
	
	bool open(size_t slice, const path_type & slice_file);
	
	bool is_open() { return ifs.is_open(); }
	
};

} // namespace stream

#endif // INNOEXTRACT_STREAM_SLICEREADER_HPP

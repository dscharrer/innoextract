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

#ifndef INNOEXTRACT_STREAM_SLICEREADER_HPP
#define INNOEXTRACT_STREAM_SLICEREADER_HPP

#include <fstream>

#include <boost/iostreams/concepts.hpp>

namespace stream {

class slice_reader : public boost::iostreams::source {
	
	std::string dir;
	std::string last_dir;
	std::string base_file;
	const uint32_t data_offset;
	const size_t slices_per_disk;
	
	size_t current_slice;
	std::string slice_file;
	uint32_t slice_size;
	
	std::ifstream ifs;
	
	bool seek(size_t slice);
	bool openFile(const std::string & file);
	
public:
	
	slice_reader(const std::string & setup_file, uint32_t data_offset);
	
	/*!
	 *   if Ver>=4107 then baseFile := PathChangeExt(PathExtractName(SetupLdrOriginalFilename), '')
	 *  else baseFile:=SetupHeader.BaseFilename;
	 */
	slice_reader(const std::string & dir, const std::string & base_file, size_t slices_per_disk);
	
	bool seek(size_t slice, uint32_t offset);
	
	std::streamsize read(char * buffer, std::streamsize bytes);
	
	inline size_t slice() { return current_slice; }
	inline std::string & file() { return slice_file; }
	
	bool open(size_t slice, const std::string & slice_file);
	
	inline bool is_open() { return ifs.is_open(); }
	
};

} // namespace stream

#endif // INNOEXTRACT_STREAM_SLICEREADER_HPP

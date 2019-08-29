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
 * Abstraction for reading the embedded or external raw setup data.
 */
#ifndef INNOEXTRACT_STREAM_SLICE_HPP
#define INNOEXTRACT_STREAM_SLICE_HPP

#include <ios>
#include <string>

#include <boost/iostreams/concepts.hpp>
#include <boost/filesystem/path.hpp>

#include "util/fstream.hpp"

namespace stream {

//! Error thrown by \ref slice_reader if there was a problem.
struct slice_error : public std::ios_base::failure {
	
	explicit slice_error(const std::string & msg) : std::ios_base::failure(msg) { }
	
};

/*!
 * Abstraction for reading either data embedded inside the setup executable or from
 * multiple external slices.
 *
 * Setup data contained in the executable is located by a non-zeore
 * \ref loader::offsets::data_offset.
 *
 * The contained data is made up of one or more \ref chunk "chunks"
 * (read by \ref chunk_reader), which in turn contain one or more  \ref file "files"
 * (read by \ref file_reader).
 */
class slice_reader : public boost::iostreams::source {
	
	typedef boost::filesystem::path path_type;
	
	// Information for reading embedded setup data
	const boost::uint32_t data_offset;
	
	// Information for eading external setup data
	path_type    dir;             //!< Slice directory specified at construction.
	std::string  base_file;       //!< Base file name for slices.
	std::string  base_file2;      //!< Fallback base filename for slices.
	const size_t slices_per_disk; //!< Number of slices grouped into each disk (for names).
	
	// Information about the current slice
	size_t          current_slice; //!< Number of the currently opened slice.
	boost::uint32_t slice_size;    //!< Size in bytes of the currently opened slice.
	
	// Streams
	util::ifstream ifs; //!< File input stream used when reading from external slices.
	std::istream * is;  //!< Input stream to read from.
	
	void seek(size_t slice);
	bool open_file(const path_type & file);
	bool open_file_case_insensitive(const path_type & dirname, const path_type & filename);
	void open(size_t slice);
	
public:
	
	static std::string slice_filename(const std::string & basename, size_t slice,
	                                  size_t slices_per_disk = 1);
	
	/*!
	 * Construct a \ref slice_reader to read from data inside the setup file.
	 * Seeking to anything except the zeroeth slice is not allowed.
	 *
	 * \param istream A seekable input stream for the setup executable.
	 *                The initial read position of the stream is ignored.
	 * \param offset  The offset within the given stream where the setup data starts.
	 *                This offset is given by \ref loader::offsets::data_offset.
	 *
	 * The constructed reader will allow reading the byte range [data_offset, file end)
	 * from the setup executable and provide this as the range [0, file end - data_offset).
	 */
	slice_reader(std::istream * istream, boost::uint32_t offset);
	
	/*!
	 * Construct a \ref slice_reader to read from external data slices (aka disks).
	 *
	 * Slice files must be located at \c $dir/$base_file-$disk.bin
	 * or \c $dir/$base_file-$disk$sliceletter.bin if \c slices_per_disk is greater
	 * than \c 1.
	 *
	 * The disk number is given by \code slice / slices_per_disk + 1 \endcode while
	 * the sliceletter is the ASCII char \code 'a' + (slice % slices_per_disk) \endcode.
	 *
	 * \param dirname          The directory containing the slice files.
	 * \param basename         The base name for slice files.
	 * \param basename2        Alternative base name for slice files.
	 * \param disk_slice_count How many slices are grouped into one disk. Must not be \c 0.
	 */
	slice_reader(const path_type & dirname, const std::string & basename, const std::string & basename2,
	             size_t disk_slice_count);
	
	/*!
	 * Attempt to seek to an offset within a slice.
	 *
	 * \param slice  The slice to seek to.
	 * \param offset The byte offset to seek to within the given slice.
	 *
	 * \return \c false if the requested slice could not be opened, or if the requested
	 *         offset is not a valid position in that slice - \c true otherwise.
	 */
	bool seek(size_t slice, boost::uint32_t offset);
	
	/*!
	 * Read a number of bytes starting at the current slice and offset within that slice.
	 *
	 * \param buffer Buffer to receive the bytes read.
	 * \param bytes  Number of bytes to read.
	 *
	 * The current offset will be advanced by the number of bytes read. It is not an error
	 * to read past the end of the current slice (unless it is the last slice). Doing so
	 * will automatically seek to the start of the next slice and continue reading from
	 * there.
	 *
	 * \return The number of bytes read or \c -1 if there was an error. Unless we are at the
	 *         end of the last slice, this function blocks until the number of requested
	 *         bytes have been read.
	 */
	std::streamsize read(char * buffer, std::streamsize bytes);
	
	//! \return the number currently opened slice.
	size_t slice() { return current_slice; }
	
	//! \return true a slice is currently open.
	bool is_open() { return (is != &ifs || ifs.is_open()); }
	
};

} // namespace stream

#endif // INNOEXTRACT_STREAM_SLICE_HPP

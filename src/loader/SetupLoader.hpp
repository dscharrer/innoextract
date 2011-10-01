
#ifndef INNOEXTRACT_LOADER_SETUPLOADER_HPP
#define INNOEXTRACT_LOADER_SETUPLOADER_HPP

#include <stddef.h>
#include <iostream>

#include "util/Types.hpp"

enum ChecksumMode {
	ChecksumAdler32,
	ChecksumCrc32
};

class SetupLoader {
	
public:
	
	struct Offsets {
		
		size_t totalSize; //!< Minimum expected size of the setup file
		
		size_t exeOffset; //!< Offset of compressed setup.e32
		size_t exeCompressedSize; //!< Size of setup.e32 after compression
		size_t exeUncompressedSize; //!< Size of setup.e32 before compression
		s32 exeChecksum; //!< Checksum of setup.e32 before compression
		ChecksumMode exeChecksumMode;  //! Type of the checksum in exeChecksum
		
		size_t messageOffset; // TODO document
		
		size_t offset0; //!< Offset of embedded setup-0.bin data
		size_t offset1; //!< Offset of embedded setup-1.bin data, or 0 when DiskSpanning=yes
		
	};
	
	/*!
	* Try to find the setup loader offsets in the given file.
	* @return true if the offsets were found, false otherwise
	*/
	static bool getOffsets(std::istream & is, Offsets & offsets);
	
private:
	
	static bool getOldOffsets(std::istream & is, Offsets & offsets);
	
	static bool getNewOffsets(std::istream & is, Offsets & offsets);
	
	static bool getOffsetsAt(std::istream & is, Offsets & offsets, size_t pos);
	
};

#endif // INNOEXTRACT_LOADER_SETUPLOADER_HPP

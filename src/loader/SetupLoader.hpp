
#ifndef INNOEXTRACT_LOADER_SETUPLOADER_HPP
#define INNOEXTRACT_LOADER_SETUPLOADER_HPP

#include <stddef.h>
#include <iostream>

#include "crypto/Checksum.hpp"

struct SetupLoader {
	
	uint32_t exeOffset; //!< Offset of compressed setup.e32. 0 means there is no exe in this file.
	uint32_t exeCompressedSize; //!< Size of setup.e32 after compression (0 = unknown)
	uint32_t exeUncompressedSize; //!< Size of setup.e32 before compression
	
	Checksum exeChecksum; //!< Checksum of setup.e32 before compression
	
	uint32_t messageOffset;
	
	/*!
		* Offset of embedded setup-0.bin data (the setup headers)
		* This points to a version string (see setup/Version.hpp) followed by a
		* compressed block of headers (see stream/BlockReader.hpp and setup/SetupHeader.hpp)
		*/
	uint32_t headerOffset;
	
	/*!
		* Offset of embedded setup-1.bin data.
		* If this is zero, the setup data is stored in seprarate files.
		*/
	uint32_t dataOffset;
	
	/*!
	* Try to find the setup loader offsets in the given file.
	*/
	void load(std::istream & is);
	
private:
	
	bool loadFromExeFile(std::istream & is);
	
	bool loadFromExeResource(std::istream & is);
	
	bool loadOffsetsAt(std::istream & is, uint32_t pos);
	
};

#endif // INNOEXTRACT_LOADER_SETUPLOADER_HPP

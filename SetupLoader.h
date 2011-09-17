
#include <stddef.h>
#include <iostream>

#include "Types.h"

enum ChecksumMode {
	ChecksumAdler32,
	ChecksumCrc32
};

class SetupLoader {
	
public:
	
	struct Offsets {
		
		size_t totalSize;
		
		size_t exeOffset;
		size_t exeCompressedSize;
		size_t exeUncompressedSize;
		s32 exeChecksum;
		ChecksumMode exeChecksumMode;
		
		size_t messageOffset;
		
		size_t offset0;
		size_t offset1;
		
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

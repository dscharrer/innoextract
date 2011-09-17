
#include "SetupLoader.h"

#include <iomanip>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "ExeReader.h"
#include "SetupLoaderFormat.h"
#include "Utils.h"

using std::cout;
using std::cerr;
using std::string;
using std::endl;
using std::setw;
using std::setfill;
using std::hex;
using std::dec;

bool SetupLoader::getOldOffsets(std::istream & is, Offsets & offsets) {
	
	SetupLoaderHeader locator;
	if(read(is.seekg(0x30), locator).fail()) {
		cerr << "error reading exe header" << endl;
		return false;
	}
	
	if(locator.id != SetupLoaderHeaderMagic) {
		cerr << "invalid exe header id: " << hex << locator.id << dec << endl;
		return false;
	}
	
	if(locator.offsetTableOffset != ~locator.notOffsetTableOffset) {
		cerr << "offset table offset mismatch" << endl;
		return false;
	}
	
	cout << "[old offsets] offset table is @ " << hex << locator.offsetTableOffset << dec << endl;
	
	return getOffsetsAt(is, offsets, locator.offsetTableOffset);
}

bool SetupLoader::getNewOffsets(std::istream & is, Offsets & offsets) {
	
	ExeReader::Resource resource = ExeReader::findResource(is, ResourceNameInstaller);
	if(!resource.offset) {
		return false;
	}
	
	cout << "[new offsets] offset table is @ " << hex << resource.offset << dec << endl;
	
	return getOffsetsAt(is, offsets, resource.offset);
}

bool SetupLoader::getOffsetsAt(std::istream & is, Offsets & offsets, size_t pos) {
	
	if(is.seekg(pos).fail()) {
		cerr << "invalid offset table offset" << endl;
		return false;
	}
	
	u32 magic;
	if(read(is, magic).fail()) {
		cerr << "error reading setup loader offset magic" << endl;
		return false;
	}
	if(magic != SetupLoaderOffsetTableMagic) {
		cerr << "invalid setup loader offset id: " << hex << magic << dec << endl;
		return false;
	}
	
	u64 bigversion;
	if(read(is, bigversion).fail()) {
		cerr << "error reading setup loader offset bigversion" << endl;
		return false;
	}
	
#ifdef HAVE_ZLIB
	u32 actual = crc32(0l, NULL, 0);
	actual = crc32(actual, reinterpret_cast<const Bytef *>(&magic), sizeof(magic));
	actual = crc32(actual, reinterpret_cast<const Bytef *>(&bigversion), sizeof(bigversion));
	u32 expected;
#endif
	
	switch(bigversion) {
		
		case SetupLoaderOffsetTableID_20: {
			
			SetupLoaderOffsetTable20 offsets20;
			if(read(is, offsets20).fail()) {
				cerr << "error reading setup loader offsets v20" << endl;
				return false;
			}
			
			offsets.totalSize = offsets20.totalSize;
			offsets.exeOffset = offsets20.exeOffset;
			offsets.exeCompressedSize = offsets20.exeCompressedSize;
			offsets.exeUncompressedSize = offsets20.exeUncompressedSize;
			offsets.exeChecksum = offsets20.exeAdler, offsets.exeChecksumMode = ChecksumAdler32;
			offsets.messageOffset = offsets20.messageOffset;
			offsets.offset0 = offsets20.offset0;
			offsets.offset1 = offsets20.offset1;
			
			return true;
		}
		
		case SetupLoaderOffsetTableID_40: {
			
			SetupLoaderOffsetTable40 offsets40;
			if(read(is, offsets40).fail()) {
				cerr << "error reading setup loader offsets v40" << endl;
				return false;
			}
			
			offsets.totalSize = offsets40.totalSize;
			offsets.exeOffset = offsets40.exeOffset;
			offsets.exeCompressedSize = offsets40.exeCompressedSize;
			offsets.exeUncompressedSize = offsets40.exeUncompressedSize;
			offsets.exeChecksum = offsets40.exeAdler, offsets.exeChecksumMode = ChecksumAdler32;
			offsets.messageOffset = 0;
			offsets.offset0 = offsets40.offset0;
			offsets.offset1 = offsets40.offset1;
			
			return true;
		}
		
		case SetupLoaderOffsetTableID_40b:
		case SetupLoaderOffsetTableID_40c: {
			
			SetupLoaderOffsetTable40b offsets40b;
			if(read(is, offsets40b).fail()) {
				cerr << "error reading setup loader offsets v40" << endl;
				return false;
			}
			
			offsets.totalSize = offsets40b.totalSize;
			offsets.exeOffset = offsets40b.exeOffset;
			offsets.exeCompressedSize = offsets40b.exeCompressedSize;
			offsets.exeUncompressedSize = offsets40b.exeUncompressedSize;
			offsets.exeChecksum = offsets40b.exeCrc, offsets.exeChecksumMode = ChecksumCrc32;
			offsets.messageOffset = 0;
			offsets.offset0 = offsets40b.offset0;
			offsets.offset1 = offsets40b.offset1;
			
#ifdef HAVE_ZLIB
			if(bigversion == SetupLoaderOffsetTableID_40c) {
				if(read(is, expected).fail()) {
					cerr << "error reading crc checksum" << endl;
					return false;
				}
				actual = crc32(actual, reinterpret_cast<const Bytef *>(&offsets40b), sizeof(offsets40b));
				break;
			}
#endif
			
			return true;
		}
		
		case SetupLoaderOffsetTableID_41: {
			
			SetupLoaderOffsetTable41 offsets41;
			if(read(is, offsets41).fail()) {
				cerr << "error reading setup loader offsets v40" << endl;
				return false;
			}
			
			offsets.totalSize = offsets41.totalSize;
			offsets.exeOffset = offsets41.exeOffset;
			offsets.exeCompressedSize = 0;
			offsets.exeUncompressedSize = offsets41.exeUncompressedSize;
			offsets.exeChecksum = offsets41.exeCrc, offsets.exeChecksumMode = ChecksumCrc32;
			offsets.messageOffset = 0;
			offsets.offset0 = offsets41.offset0;
			offsets.offset1 = offsets41.offset1;
			
#ifdef HAVE_ZLIB
			expected = offsets41.tableCrc;
			actual = crc32(actual, reinterpret_cast<const Bytef *>(&offsets41), sizeof(offsets41) - sizeof(s32));
			break;
#else
			return true;
#endif
		}
		
		case SetupLoaderOffsetTableID_51: {
			
			SetupLoaderOffsetTable51 offsets51;
			if(read(is, offsets51).fail()) {
				cerr << "error reading setup loader offsets v40" << endl;
				return false;
			}
			
			if(offsets51.version != 1) {
				cerr << "warning: unexpected setup loader offset table version: " << offsets51.version << endl;
			}
			
			offsets.totalSize = offsets51.totalSize;
			offsets.exeOffset = offsets51.exeOffset;
			offsets.exeCompressedSize = 0;
			offsets.exeUncompressedSize = offsets51.exeUncompressedSize;
			offsets.exeChecksum = offsets51.exeCrc, offsets.exeChecksumMode = ChecksumCrc32;
			offsets.messageOffset = 0;
			offsets.offset0 = offsets51.offset0;
			offsets.offset1 = offsets51.offset1;
			
#ifdef HAVE_ZLIB
			expected = offsets51.tableCrc;
			actual = crc32(actual, reinterpret_cast<const Bytef *>(&offsets51), sizeof(offsets51) - sizeof(s32));
			break;
#else
			return true;
#endif
		}
		
		default: {
			cerr << "unsupported setup loader offset table version: " << hex << bigversion << dec << endl;
			return false;
		}
		
	}
	
#ifdef HAVE_ZLIB
	if(actual != expected) {
		cerr << "CRC32 mismatch in setup loader offsets" << endl;
		return false;
	}
	
	return true;
#endif
}

bool SetupLoader::getOffsets(std::istream & is, Offsets & offsets) {
	
	if(getOldOffsets(is, offsets)) {
		return true;
	}
	
	return getNewOffsets(is, offsets);
}

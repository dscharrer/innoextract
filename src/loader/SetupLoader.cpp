
#include <loader/SetupLoader.hpp>

#include <iomanip>

#include <lzma.h>

#include "loader/ExeReader.hpp"
#include "loader/SetupLoaderFormat.hpp"
#include "util/Output.hpp"
#include "util/Utils.hpp"

using std::cout;
using std::string;
using std::endl;
using std::setw;
using std::setfill;
using std::hex;
using std::dec;

bool SetupLoader::getOldOffsets(std::istream & is, Offsets & offsets) {
	
	SetupLoaderHeader locator;
	if(read(is.seekg(0x30), locator).fail()) {
		error << "error reading exe header";
		return false;
	}
	
	if(locator.id != SetupLoaderHeaderMagic) {
		cout << "invalid exe header id: " << hex << locator.id << dec << endl;
		return false;
	}
	
	if(locator.offsetTableOffset != ~locator.notOffsetTableOffset) {
		cout << "offset table offset mismatch" << endl;
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
		error << "invalid offset table offset";
		return false;
	}
	
	u32 magic;
	if(read(is, magic).fail()) {
		error << "error reading setup loader offset magic";
		return false;
	}
	if(magic != SetupLoaderOffsetTableMagic) {
		error << "invalid setup loader offset id: " << hex << magic << dec;
		return false;
	}
	
	u64 bigversion;
	if(read(is, bigversion).fail()) {
		error << "error reading setup loader offset bigversion";
		return false;
	}
	
	u32 actual = lzma_crc32(reinterpret_cast<const uint8_t *>(&magic), sizeof(magic), 0);
	actual = lzma_crc32(reinterpret_cast<const uint8_t *>(&bigversion), sizeof(bigversion), actual);
	u32 expected;
	
	switch(bigversion) {
		
		case SetupLoaderOffsetTableID_10: {
			
			SetupLoaderOffsetTable10 offsets20;
			if(read(is, offsets20).fail()) {
				error << "error reading setup loader offsets v20";
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
				error << "error reading setup loader offsets v40";
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
				error << "error reading setup loader offsets v40";
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
			
			if(bigversion == SetupLoaderOffsetTableID_40c) {
				if(read(is, expected).fail()) {
					error << "error reading crc checksum";
					return false;
				}
				actual = lzma_crc32(reinterpret_cast<const uint8_t *>(&offsets40b), sizeof(offsets40b), actual);
				break;
			}
			
			return true;
		}
		
		case SetupLoaderOffsetTableID_41: {
			
			SetupLoaderOffsetTable41 offsets41;
			if(read(is, offsets41).fail()) {
				error << "error reading setup loader offsets v40";
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
			
			expected = offsets41.tableCrc;
			actual = lzma_crc32(reinterpret_cast<const uint8_t *>(&offsets41), sizeof(offsets41) - sizeof(s32), actual);
			break;
		}
		
		case SetupLoaderOffsetTableID_51: {
			
			SetupLoaderOffsetTable51 offsets51;
			if(read(is, offsets51).fail()) {
				error << "error reading setup loader offsets v40";
				return false;
			}
			
			if(offsets51.version != 1) {
				error << "warning: unexpected setup loader offset table version: " << offsets51.version;
			}
			
			offsets.totalSize = offsets51.totalSize;
			offsets.exeOffset = offsets51.exeOffset;
			offsets.exeCompressedSize = 0;
			offsets.exeUncompressedSize = offsets51.exeUncompressedSize;
			offsets.exeChecksum = offsets51.exeCrc, offsets.exeChecksumMode = ChecksumCrc32;
			offsets.messageOffset = 0;
			offsets.offset0 = offsets51.offset0;
			offsets.offset1 = offsets51.offset1;
			
			expected = offsets51.tableCrc;
			actual = lzma_crc32(reinterpret_cast<const uint8_t *>(&offsets51), sizeof(offsets51) - sizeof(s32), actual);
			break;
		}
		
		default: {
			error << "unsupported setup loader offset table version: " << hex << bigversion << dec;
			return false;
		}
		
	}
	
	if(actual != expected) {
		error << "CRC32 mismatch in setup loader offsets";
		return false;
	} else {
		cout << "setup loader offset table CRC32 match" << endl;
	}
	
	return true;
}

bool SetupLoader::getOffsets(std::istream & is, Offsets & offsets) {
	
	if(getOldOffsets(is, offsets)) {
		return true;
	}
	
	return getNewOffsets(is, offsets);
}

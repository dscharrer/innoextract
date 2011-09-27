
#include "Types.h"

#pragma pack(push,1)

const int ResourceNameInstaller = 11111;

const u32 SetupLoaderOffsetTableMagic = 0x506c4472;
const u32 SetupLoaderHeaderMagic = 0x6f6e6e49;

enum SetupLoaderOffsetTableID {
	SetupLoaderOffsetTableID_10  = 0x7856658732305374l,
	SetupLoaderOffsetTableID_40  = 0x7856658734305374l,
	SetupLoaderOffsetTableID_40b = 0x7856658735305374l,
	SetupLoaderOffsetTableID_40c = 0x7856658736305374l,
	SetupLoaderOffsetTableID_41  = 0x7856658737305374l,
	SetupLoaderOffsetTableID_51  = 0x2a0b7bd7e6cd5374l,
};

//  2.0.8, 2.0.11, 2.0.17, 2.0.18
//  3.0.0, 3.0.1, 3.0.2, 3.0.3, 3.0.4, 3.0.5, 3.0.6, 3.0.7, 3.0.8
struct SetupLoaderOffsetTable10 {
	s32 totalSize;
	s32 exeOffset;
	s32 exeCompressedSize;
	s32 exeUncompressedSize;
	s32 exeAdler;
	s32 messageOffset;
	s32 offset0;
	s32 offset1;
};

//  4.0.0, 4.0.1, 4.0.2
//! removed messageOffset
struct SetupLoaderOffsetTable40 {
	s32 totalSize;
	s32 exeOffset;
	s32 exeCompressedSize;
	s32 exeUncompressedSize;
	s32 exeAdler;
	s32 offset0;
	s32 offset1;
};

//  4.0.3, 4.0.4, 4.0.5, 4.0.6, 4.0.7, 4.0.8, 4.0.9
//! changed exeAdler -> exeCrc
struct SetupLoaderOffsetTable40b {
	s32 totalSize;
	s32 exeOffset;
	s32 exeCompressedSize;
	s32 exeUncompressedSize;
	s32 exeCrc;
	s32 offset0;
	s32 offset1;
};

//  4.0.10, 4.0.11
//  4.1.0, 4.1.1, 4.1.2, 4.1.3, 4.1.4, 4.1.5
//! added tableCrc
struct SetupLoaderOffsetTable40c : public SetupLoaderOffsetTable40b {
	s32 tableCrc; //!< CRC32 of all prior fields in this structure
};

//  4.1.6, 4.1.7, 4.1.8
//  4.2.0, 4.2.1, 4.2.2, 4.2.3, 4.2.4, 4.2.5, 4.2.6, 4.2.7
//  5.0.0, 5.0.1, 5.0.2, 5.0.3, 5.0.4
//  5.1.0, 5.1.2
//! removed exeCompressedSize
struct SetupLoaderOffsetTable41 {
	s32 totalSize;
	s32 exeOffset;
	s32 exeUncompressedSize;
	s32 exeCrc;
	s32 offset0;
	s32 offset1;
	s32 tableCrc; //!< CRC32 of all prior fields in this structure
};

//  5.1.5, 5.1.7, 5.1.10, 5.1.13
//  5.2.0, 5.2.1, 5.2.3, 5.2.5
//  5.3.0, 5.3.3, 5.3.5, 5.3.6, 5.3.7, 5.3.8, 5.3.9, 5.3.10
//! added version
struct SetupLoaderOffsetTable51 {
	u32 version; //!< = 1
	u32 totalSize;
	u32 exeOffset;
	u32 exeUncompressedSize;
	s32 exeCrc;
	u32 offset0;
	u32 offset1;
	s32 tableCrc; //!< CRC32 of all prior fields in this structure
};

//  2.0.8, 2.0.11, 2.0.17, 2.0.18
//  3.0.0, 3.0.1, 3.0.2, 3.0.3, 3.0.4, 3.0.5, 3.0.6, 3.0.7, 3.0.8
//  4.0.0, 4.0.1, 4.0.2, 4.0.3, 4.0.4, 4.0.5, 4.0.6, 4.0.7, 4.0.8, 4.0.9, 4.0.10, 4.0.11
//  4.1.0, 4.1.1, 4.1.2, 4.1.3, 4.1.4, 4.1.5, 4.1.6, 4.1.7, 4.1.8
//  4.2.0, 4.2.1, 4.2.2, 4.2.3, 4.2.4, 4.2.5, 4.2.6, 4.2.7
//  5.0.0, 5.0.1, 5.0.2, 5.0.3, 5.0.4
//  5.1.0, 5.1.2
// This has been replaced with a .exe data resource in newer versions.
struct SetupLoaderHeader {
	
	s32 id; //!< = SetupLoaderHeaderMagic
	
	s32 offsetTableOffset;
	
	s32 notOffsetTableOffset; //!< bitwise negated value of offsetTableOffset
	
};

#pragma pack(pop)

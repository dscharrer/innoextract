
#include "Types.h"

# pragma pack(push, 1)

struct CoffFileHeader {
	
	u16 machine; // ignored
	
	u16 nsections;
	
	u32 creationTime; // ignored
	
	u32 symbolTableOffset;
	u32 nsymbols;
	
	u16 optionalHeaderSize;
	
	u16 characteristics; // ignored
	
};

struct DataDirectory {
	
	u32 address;
	
	u32 size;
	
};

struct ResourceTable {
	
	u32 characteristics;
	
	u32 timestamp;
	
	u16 majorVersion;
	u16 minorVersion;
	
	u16 nbnames;
	u16 nbids;
	
};

struct ResourceEntry {
	
	u32 id;
	u32 offset;
	
};

struct ResourceLeaf {
	
	u32 address;
	u32 size;
	
	u32 codepage;
	
	u32 reserved;
	
};

struct CoffSection {
	
	char name[8];
	
	u32 virtualSize;
	u32 virtualAddress;
	
	u32 rawSize;
	u32 rawAddress;
	
	u32 relocationAddress;
	u32 linenumberAddress;
	u16 relocationCount;
	u16 linenumberCount;
	
	u32 characteristics;
	
};

#pragma pack(pop)


#include "Types.h"

# pragma pack(push, 1)

struct CoffFileHeader {
	
	u16 machine; // ignored
	
	//! Number of CoffSection structures following this header after optionalHeaderSize bytes.
	u16 nsections;
	
	u32 creationTime; // ignored
	
	u32 symbolTableOffset; // ignored
	u32 nsymbols; // ignored
	
	//! Offset from the end of this header to the start of the section table.
	u16 optionalHeaderSize;
	
	u16 characteristics; // ignored
	
};

struct CoffDataDirectory {
	
	/*!
	 * Virtual memory address of the start of this dictionary.
	 * Use the section table to map this to a file offset.
	 */
	u32 address;
	
	//! Size of this data directory.
	u32 size;
	
};

struct CoffResourceTable {
	
	u32 characteristics; // ignored
	
	u32 timestamp; // ignored
	
	u16 majorVersion; // ignored
	u16 minorVersion; // minorVersion
	
	u16 nbnames; //!< Number of named resource entries.
	u16 nbids; //! Number of id resource entries.
	
	//! Folowed by nbames CoffResourceEntry entries and the another nbids CoffResourceEntry.
};

struct CoffResourceEntry {
	
	u32 id; //!< Entry ID.
	
	/*!
	 * Highest order bit: 1 = points to another CoffResourceTable / 0 = points to a CoffResourceLeaf
	 * Remaining 31 bits: Offset to the CoffResourceTable CoffResourceLeaf relative to the directory start.
	 */
	u32 offset;
	
};

struct CoffResourceLeaf {
	
	u32 address; //! Virtual memory address of the resource data.
	u32 size; //! Size of the resource data.
	
	u32 codepage; //! Windows codepage for the resource encoding.
	
	u32 reserved; // ignored
	
};

struct CoffSection {
	
	char name[8]; // ignored
	
	u32 virtualSize; //!< Section size in virtual memory.
	u32 virtualAddress; //!< Base virtual memory address.
	
	u32 rawSize; //!< Section size in the file.
	u32 rawAddress; //!< Base file offset.
	
	u32 relocationAddress; // ignored
	u32 linenumberAddress; // ignored
	u16 relocationCount; // ignored
	u16 linenumberCount; // ignored
	
	u32 characteristics; // ignored
	
};

#pragma pack(pop)

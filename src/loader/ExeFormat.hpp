
#ifndef INNOEXTRACT_LOADER_EXEFORMAT_HPP
#define INNOEXTRACT_LOADER_EXEFORMAT_HPP

#include <stdint.h>

# pragma pack(push, 1)

struct CoffFileHeader {
	
	uint16_t machine; // ignored
	
	//! Number of CoffSection structures following this header after optionalHeaderSize bytes.
	uint16_t nsections;
	
	uint32_t creationTime; // ignored
	
	uint32_t symbolTableOffset; // ignored
	uint32_t nsymbols; // ignored
	
	//! Offset from the end of this header to the start of the section table.
	uint16_t optionalHeaderSize;
	
	uint16_t characteristics; // ignored
	
};

struct CoffDataDirectory {
	
	/*!
	 * Virtual memory address of the start of this dictionary.
	 * Use the section table to map this to a file offset.
	 */
	uint32_t address;
	
	//! Size of this data directory.
	uint32_t size;
	
};

struct CoffResourceTable {
	
	uint32_t characteristics; // ignored
	
	uint32_t timestamp; // ignored
	
	uint16_t majorVersion; // ignored
	uint16_t minorVersion; // minorVersion
	
	uint16_t nbnames; //!< Number of named resource entries.
	uint16_t nbids; //! Number of id resource entries.
	
	//! Folowed by nbames CoffResourceEntry entries and the another nbids CoffResourceEntry.
};

struct CoffResourceEntry {
	
	uint32_t id; //!< Entry ID.
	
	/*!
	 * Highest order bit: 1 = points to another CoffResourceTable / 0 = points to a CoffResourceLeaf
	 * Remaining 31 bits: Offset to the CoffResourceTable CoffResourceLeaf relative to the directory start.
	 */
	uint32_t offset;
	
};

struct CoffResourceLeaf {
	
	uint32_t address; //! Virtual memory address of the resource data.
	uint32_t size; //! Size of the resource data.
	
	uint32_t codepage; //! Windows codepage for the resource encoding.
	
	uint32_t reserved; // ignored
	
};

struct CoffSection {
	
	char name[8]; // ignored
	
	uint32_t virtualSize; //!< Section size in virtual memory.
	uint32_t virtualAddress; //!< Base virtual memory address.
	
	uint32_t rawSize; //!< Section size in the file.
	uint32_t rawAddress; //!< Base file offset.
	
	uint32_t relocationAddress; // ignored
	uint32_t linenumberAddress; // ignored
	uint16_t relocationCount; // ignored
	uint16_t linenumberCount; // ignored
	
	uint32_t characteristics; // ignored
	
};

#pragma pack(pop)

#endif // INNOEXTRACT_LOADER_EXEFORMAT_HPP

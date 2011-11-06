
#ifndef INNOEXTRACT_LOADER_EXEREADER_HPP
#define INNOEXTRACT_LOADER_EXEREADER_HPP

#include <stdint.h>
#include <istream>
#include <vector>

/*!
 * Minimal PE/COFF parser that can find resources by ID in .exe files.
 * This implementation is optimized to look for exactly one resource.
 */
class ExeReader {
	
public:
	
	struct Resource {
		
		uint32_t offset;
		
		uint32_t size;
		
	};
	
	enum ResourceID {
		TypeData = 10,
		LanguageDefault = 0
	};
	
	/*!
	 * Find where a resource with a given ID is stored in a MS PE/COFF executable.
	 * @return The location of the resource or (0, 0) if the requested resource does not exist.
	 */
	static Resource findResource(std::istream & is, uint32_t name, uint32_t type = TypeData,
	                             uint32_t language = LanguageDefault);
	
private:
	
	struct CoffFileHeader;
	struct CoffSection;
	
	typedef std::vector<CoffSection> CoffSectionTable;
	
	/*!
	 * Find the entry in a resource table with a given ID.
	 * The input stream is expected to be positioned at the start of the table.
	 * The position if the stream after the function call is undefined.
	 * 
	 * @return:
	 *   Highest order bit: 1 = points to another CoffResourceTable
	 *                      0 = points to a CoffResourceLeaf
	 *   Remaining 31 bits: Offset to the CoffResourceTable CoffResourceLeaf relative to
	 *                      the directory start.
	 */
	static uint32_t findResourceEntry(std::istream & ifs, uint32_t id);
	
	static bool loadSectionTable(std::istream & ifs, uint32_t peOffset,
	                             const CoffFileHeader & coff, CoffSectionTable & table);
	
	static uint32_t memoryAddressToFileOffset(const CoffSectionTable & sections, uint32_t memory);
	
};

#endif // INNOEXTRACT_LOADER_EXEREADER_HPP

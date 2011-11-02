
#ifndef INNOEXTRACT_LOADER_EXEREADER_HPP
#define INNOEXTRACT_LOADER_EXEREADER_HPP

#include <stddef.h>
#include <istream>
#include <vector>

/*!
 * Minimal PE/COFF parser that can find resources by ID in .exe files.
 * This implementation is optimized to look for exactly one resource.
 */
class ExeReader {
	
public:
	
	struct Resource {
		
		size_t offset;
		
		size_t size;
		
	};
	
	enum ResourceID {
		TypeData = 10,
		LanguageDefault = 0
	};
	
	/*!
	 * Find where a resource with a given ID is stored in a MS PE/COFF executable.
	 * @return The location of the resource or (0, 0) if the requested resource does not exist.
	 */
	static Resource findResource(std::istream & is, int name, int type = TypeData, int language = LanguageDefault);
	
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
	static size_t findResourceEntry(std::istream & ifs, int id);
	
	static bool loadSectionTable(std::istream & ifs, size_t peOffset, const CoffFileHeader & coff, CoffSectionTable & table);
	
	static size_t memoryAddressToFileOffset(const CoffSectionTable & sections, size_t memory);
	
};

#endif // INNOEXTRACT_LOADER_EXEREADER_HPP

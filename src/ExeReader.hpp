
#ifndef INNOEXTRACT_EXEREADER_HPP
#define INNOEXTRACT_EXEREADER_HPP

#include <stddef.h>
#include <istream>
#include <vector>

struct CoffFileHeader;
struct CoffSection;

/*!
 * PE/COFF parser that can find resources by ID in .EXE files.
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
	
	typedef std::vector<CoffSection> CoffSectionTable;
	
	static size_t findResourceEntry(std::istream & ifs, int id);
	
	static bool loadSectionTable(std::istream & ifs, size_t peOffset, const CoffFileHeader & coff, CoffSectionTable & table);
	
	static size_t memoryAddressToFileOffset(const CoffSectionTable & sections, size_t memory);
	
};

#endif // INNOEXTRACT_EXEREADER_HPP

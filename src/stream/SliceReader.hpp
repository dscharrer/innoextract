
#ifndef INNOEXTRACT_STREAM_SLICEREADER_HPP
#define INNOEXTRACT_STREAM_SLICEREADER_HPP

#include <fstream>

#include <boost/iostreams/concepts.hpp>

class SliceReader : public boost::iostreams::source {
	
	std::string dir;
	std::string lastDir;
	std::string baseFile;
	const uint32_t dataOffset;
	const size_t slicesPerDisk;
	
	size_t currentSlice;
	std::string sliceFile;
	uint32_t sliceSize;
	
	std::ifstream ifs;
	
	bool seek(size_t slice);
	bool openFile(const std::string & file);
	
public:
	
	SliceReader(const std::string & setupFile, uint32_t dataOffset);
	
	/*!
	 *   if Ver>=4107 then baseFile := PathChangeExt(PathExtractName(SetupLdrOriginalFilename), '')
	 *  else baseFile:=SetupHeader.BaseFilename;
	 */
	SliceReader(const std::string & dir, const std::string & baseFile, size_t slicesPerDisk);
	
	bool seek(size_t slice, uint32_t offset);
	
	std::streamsize read(char * buffer, std::streamsize bytes);
	
	inline size_t slice() { return currentSlice; }
	inline std::string & file() { return sliceFile; }
	
	bool open(size_t slice, const std::string & sliceFile);
	
	inline bool isOpen() { return ifs.is_open(); }
	
};

#endif // INNOEXTRACT_STREAM_SLICEREADER_HPP

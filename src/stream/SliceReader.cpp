
#include "stream/SliceReader.hpp"

#include <stdint.h>
#include <sstream>
#include <cstring>
#include <limits>

#include "util/color.hpp"
#include "util/load.hpp"
#include "util/util.hpp"

using std::string;

namespace {

const char sliceIds[][8] = {
	{ 'i', 'd', 's', 'k', 'a', '1', '6', 0x1a },
	{ 'i', 'd', 's', 'k', 'a', '3', '2', 0x1a },
};

} // anonymous namespace

SliceReader::SliceReader(const string & setupFile, uint32_t _dataOffset)
	: dir(), lastDir(), baseFile(), dataOffset(_dataOffset), slicesPerDisk(1),
	  currentSlice(0) {
	
	ifs.open(setupFile.c_str(), std::ios_base::binary | std::ios_base::in | std::ios_base::ate);
	
	sliceSize = uint32_t(std::min<std::streampos>(ifs.tellg(), std::numeric_limits<int32_t>::max()));
	if(ifs.seekg(dataOffset).fail()) {
		ifs.close();
	}
}

SliceReader::SliceReader(const std::string & _dir, const std::string & _baseFile,
                         size_t _slicesPerDisk)
	: dir(_dir), lastDir(_dir), baseFile(_baseFile), dataOffset(0), slicesPerDisk(_slicesPerDisk),
	  currentSlice(0) { }

bool SliceReader::seek(size_t slice) {
	
	if(slice == currentSlice && ifs.is_open()) {
		return true;
	}
	
	if(dataOffset != 0) {
		LogError << "[slice] cannot change slices in single-file setup";
		return false;
	}
	
	return open(slice, string());
}

bool SliceReader::openFile(const std::string & file) {
	
	std::cout << color::cyan << "\33[2K\r[slice] opening " << file << color::reset << std::endl;
	
	ifs.close();
	
	ifs.open(file.c_str(), std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	if(ifs.fail()) {
		return false;
	}
	
	std::streampos fileSize = ifs.tellg();
	ifs.seekg(0);
	
	char magic[8];
	if(ifs.read(magic, 8).fail()) {
		ifs.close();
		LogError << "[slice] error reading magic number";
		return false;
	}
	bool found = false;
	for(size_t i = 0; ARRAY_SIZE(sliceIds); i++) {
		if(!std::memcmp(magic, sliceIds[i], 8)) {
			found = true;
			break;
		}
	}
	if(!found) {
		LogError << "[slice] bad magic number";
		ifs.close();
		return false;
	}
	
	sliceSize = load_number<uint32_t>(ifs);
	if(ifs.fail() || std::streampos(sliceSize) > fileSize) {
		LogError << "[slice] bad slice size: " << sliceSize << " > " << fileSize;
		ifs.close();
		return false;
	}
	
	sliceFile = file;
	
	size_t dirpos = file.find_last_of('/');
	if(dirpos == string::npos) {
		lastDir.clear();
	} else {
		lastDir = file.substr(0, dirpos + 1);
	}
	
	return true;
}

bool SliceReader::open(size_t slice, const std::string & file) {
	
	currentSlice = slice;
	ifs.close();
	
	if(slicesPerDisk == 0) {
		LogError << "[slice] slices per disk must not be zero";
		return false;
	}
	
	std::string sliceFile = file;
	
	if(sliceFile.empty()) {
		
		std::ostringstream oss;
		oss << baseFile << '-';
		if(slicesPerDisk == 1) {
			oss << (slice + 1);
		} else {
			size_t major = (slice / slicesPerDisk) + 1;
			size_t minor = slice % slicesPerDisk;
			oss << major << char(uint8_t('a') + minor);
		}
		oss << ".bin";
		
		sliceFile = oss.str();
	}
	
	if(sliceFile[0] == '/') {
		return openFile(sliceFile);
	}
	
	if(openFile(lastDir + sliceFile)) {
		return true;
	}
	
	if(dir != lastDir && openFile(dir + sliceFile)) {
		return true;
	}
	
	return false;
}

bool SliceReader::seek(size_t slice, uint32_t offset) {
	
	if(!seek(slice)) {
		return false;
	}
	
	if(offset > sliceSize - dataOffset) {
		return false;
	}
	
	if(ifs.seekg(dataOffset + offset).fail()) {
		return false;
	}
	
	return true;
}

std::streamsize SliceReader::read(char * buffer, std::streamsize bytes) {
	
	std::streamsize nread = 0;
	
	if(!seek(currentSlice)) {
		return nread;
	}
	
	while(bytes > 0) {
		
		std::streamsize remaining = std::streamsize(sliceSize - size_t(ifs.tellg()));
		if(!remaining) {
			if(!seek(currentSlice + 1)) {
				return nread;
			}
			remaining = std::streamsize(sliceSize - size_t(ifs.tellg()));
		}
		
		std::streamsize read = ifs.read(buffer, std::min(remaining, bytes)).gcount();
		
		nread += read, buffer += read, bytes -= read;
	}
	
	return nread;
}

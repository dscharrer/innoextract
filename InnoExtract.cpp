
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <vector>

#include "SetupLoader.h"
#include "Types.h"
#include "Utils.h"

using std::cout;
using std::cerr;
using std::string;
using std::endl;
using std::setw;
using std::setfill;
using std::hex;
using std::dec;

int main(int argc, char * argv[]) {
	
	if(argc <= 1) {
		cerr << "usage: innoextract <Inno Setup installer>" << endl;
		return 1;
	}
	
	std::ifstream ifs(argv[1], strm::in | strm::binary | strm::ate);
	
	if(!ifs.is_open()) {
		cerr << "error opening file" << endl;
		return 1;
	}
	
	size_t fileSize = ifs.tellg();
	if(!fileSize) {
		cerr << "cannot read file or empty file" << endl;
		return 1;
	}
	
	SetupLoader::Offsets offsets;
	if(!SetupLoader::getOffsets(ifs, offsets)) {
		cerr << "failed to load setup loader offsets" << endl;
		return 1;
	}
	
	cout << "loaded offsets:" << endl;
	cout << "- total size: " << offsets.totalSize << endl;
	cout << "- exe: @ " << hex << offsets.exeOffset << dec << "  compressed: " << offsets.exeCompressedSize << "  uncompressed: " << offsets.exeUncompressedSize << endl;
	cout << "- exe checksum: " << hex << setfill('0') << setw(8) << offsets.exeChecksum << dec << " (" << (offsets.exeChecksumMode == ChecksumAdler32 ? "Alder32" : "CRC32") << ')' << endl;
	cout << "- messageOffset: " << hex << offsets.messageOffset << dec << endl;
	cout << "- offset:  0: " << hex << offsets.offset0 << "  1: " << offsets.messageOffset << dec << endl;
	
	ifs.seekg(offsets.offset0);
	
	char version[64];
	if(read(ifs, version).fail()) {
		cerr << "error reading version!";
		return 1;
	}
	cout << "version: \"" << safestring(version) << '"' << endl;
	
	return 0;
}

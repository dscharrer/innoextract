
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <bitset>

#include <lzma.h>

#include "Types.h"
#include "SetupLoader.hpp"
#include "Utils.hpp"

#include "BlockReader.hpp"

using std::cout;
using std::cerr;
using std::string;
using std::endl;
using std::setw;
using std::setfill;
using std::hex;
using std::dec;

#pragma pack(push,1)

struct BlockHeader {
	u32 storedSize; // Total bytes written, including the CRCs
	u8 compressed; // True if data is compressed, False if not
};

#pragma pack(pop)

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
	
	u64 fileSize = ifs.tellg();
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
		cerr << "error reading version!" << endl;
		return 1;
	}
	cout << "version: \"" << safestring(version) << '"' << endl;
	
	std::istream * _is = BlockReader::get(ifs);
	if(!_is) {
		cerr << "error reading block" << endl;
		return 1;
	}
	std::istream & is = *_is;
	
	std::string strings[29];
	
	for(size_t i = 0; i < sizeof(strings)/sizeof(*strings); i++) {
		
		u32 size;
		if(read(is, size).fail()) {
			cerr << "error reading string size #" << i << endl;
			return 1;
		}
		
		strings[i].resize(size);
		
		if(is.read(&strings[i][0], size).fail()) {
			cerr << "error reading string #" << i << endl;
			return 1;
		}
		
	}
	
	const char * names[29] = {
		"App Name", "App Ver Name", "App Id", "Copyright", "Publisher", "Publisher URL",
		"SupportPhone", "Support URL", "Updates URL", "Version", "Default Dir Name",
		"Default Group Name", "Base Filename", "License Text",
		"Info Before Text", "Info After Text", "Uninstall Files Dir", "Uninstall Display Name",
		"Uninstall Display Icon", "App Mutex", "Default User Info Name",
		"Default User Info Org", "Default User Info Serial", "Compiled Code Text",
		"Readme", "Contact", "Comments", "App Modify Path",
		"Signed Uninstaller Signature"
	};
	
	for(size_t i = 0; i < sizeof(strings)/sizeof(*strings); i++) {
		if(i != 23) {
			cout << "- " << names[i] << ": \"" << strings[i] << '"' << endl;
		} else {
			cout << "- " << names[i] << ": " << strings[i].length() << " bytes" << endl;
		}
	}
	
	
	delete _is;
	
	
	return 0;
}


#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <bitset>

#include <zlib.h>
#include <lzma.h>

#include <boost/iostreams/filter/zlib.hpp>

#include "Types.h"
#include "SetupLoader.hpp"
#include "Utils.hpp"

#include "liblzmadec/lzmadec.h"
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

template<class T>
T ceildiv(T num, T div) {
	return (num + div - 1) / div;
}

static bool lzmadec_header_properties(u8 * pb, u8 * lp, u8 * lc, const u8 c) {
	
	/* pb, lp and lc are encoded into a single byte. */
	if(c > (9 * 5 * 5)) {
		return LZMADEC_HEADER_ERROR;
	}
	
	*pb = c / (9 * 5);        /* 0 <= pb <= 4 */
	*lp = (c % (9 * 5)) / 9;  /* 0 <= lp <= 4 */
	*lc = c % 9;              /* 0 <= lc <= 8 */
	
	return true;
}


/* Parse the dictionary size (4 bytes, little endian) */
static bool lzmadec_header_dictionary(u32 * size, const u8 * buffer) {
	
	*size = 0;
	for(size_t i = 0; i < 4; i++) {
		*size += u32(*buffer++) << (i * 8);
	}
	
	/* The dictionary size is limited to 256 MiB (checked from
	   LZMA SDK 4.30) */
	return (*size <= (1 << 28));
}

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
	
	//std::ofstream ofs("setupinfo.txt", strm::binary | strm::trunc | strm::out);
	
	//ofs << is->rdbuf();
	
	/*
	try {
		
		while(!is->eof()) {
			
			char buf[1024];
			std::streamsize n = is->read(buf, sizeof(buf)).gcount();
			
			ofs.write(buf, n);
			
		}
		
		
	} catch(std::string & str) {
		cerr << "bad: " << str;
	} catch(...) {
		cerr << "error reading stream";
	}*/
	
	delete _is;
	
	/*
	
	u32 crc;
	BlockHeader block;
	if(read(read(ifs, crc), block).fail()) {
		cerr << "error reading block header" << endl;
		return 1;
	}
	
	u32 actual = crc32(crc32(0l, NULL, 0), reinterpret_cast<const Bytef *>(&block), sizeof(block));
	if(crc != actual) {
		cerr << "block header CRC32 mismatch" << endl;
		return 1;
	} else {
		cout << "block header CRC32 match" << endl;
	}
	
	cout << "block size: " << block.storedSize << "  compressed: " << int(block.compressed) << endl;
	
	if(u64(ifs.tellg()) + u64(block.storedSize) > fileSize) {
		cerr << "block truncated" << endl;
		return 1;
	}
	
	lzma_stream stream = LZMA_STREAM_INIT;
	int_fast8_t lzma_ret;
	stream.allocator = NULL;
	
	std::ofstream ofs("setupinfo.txt", strm::binary | strm::trunc | strm::out);
	
	bool first = true;
	u64 inbytes = block.storedSize;
	while(inbytes) {
		
		if(inbytes < sizeof(u32) + 1) {
			cerr << "bad block length!" << endl;
			return 1;
		}
		
		u32 chunkCrc32;
		if(read(ifs, chunkCrc32).fail()) {
			cerr << "error reading chunk CRC32" << endl;
			return 1;
		}
		inbytes -= sizeof(chunkCrc32);
		
		char chunk[4096];
		size_t length = size_t(std::min(u64(sizeof(chunk)), inbytes));
		if(ifs.read(chunk, length).fail()) {
			cerr << "error reading chunk" << endl;
			return 1;
		}
		inbytes -= length;
		
		u32 actual = crc32(crc32(0l, NULL, 0), reinterpret_cast<const Bytef *>(chunk), length);
		if(chunkCrc32 != actual) {
			cerr << "chunk CRC32 mismatch" << endl;
			return 1;
		} else {
			cout << "chunk CRC32 match" << endl;
		}
		
		if(block.compressed) {
			
			stream.avail_in = length;
			stream.next_in = reinterpret_cast<uint8_t *>(chunk);
			
			if(first) {
				first = false;
				
				u8 lc, lp, pb;
				if(!lzmadec_header_properties(&pb, &lp, &lc, stream.next_in[0])) {
					cerr << "[lzma] invalid header" << endl;
					return 1;
				}
				stream.next_in += 1, stream.avail_in -= 1;
				
				u32 dictSize;
				if(!lzmadec_header_dictionary(&dictSize, stream.next_in)) {
					cerr << "[lzma] invalid dictionary size" << endl;
				}
				stream.next_in += 4, stream.avail_in -= 4;
				
				cout << "[lzma] lc=" << int(lc) << "  lp=" << int(lp) << "  pb=" << int(pb) << "  dict size: " << dictSize << " (" << std::bitset<32>(dictSize) << ')' << endl;
				
				lzma_options_lzma options;
				memset(&options, 0, sizeof(options));
				options.lc = lc, options.lp = lp, options.pb = pb;
				options.dict_size = dictSize;
				options.preset_dict = NULL;
				
				lzma_filter filters[2] = { { LZMA_FILTER_LZMA1,  &options }, { LZMA_VLI_UNKNOWN } };
				
				lzma_ret = lzma_raw_decoder(&stream, filters);
				if(lzma_ret != LZMADEC_OK) {
					cerr << "error initializing LZMA stream" << endl;
					return 1;
				}
				
			}
			
			do {
				
				size_t last_in = stream.avail_in;
				
				char buffer[8024];
				
				stream.avail_out = sizeof(buffer);
				stream.next_out = reinterpret_cast<uint8_t *>(buffer);
				
				lzma_ret = lzma_code(&stream, (inbytes == 0) ? LZMA_FINISH : LZMA_RUN);
				
				cout << "[lzma] " << (last_in - stream.avail_in) << " -> " << (sizeof(buffer) - stream.avail_out) << endl;
				
				ofs.write(buffer, sizeof(buffer) - stream.avail_out);
				
			} while((lzma_ret == LZMA_BUF_ERROR || lzma_ret == LZMA_OK) && (stream.avail_in || !stream.avail_out));
			
			if(lzma_ret != LZMA_OK && lzma_ret != LZMA_STREAM_END && lzma_ret != LZMA_BUF_ERROR) {
				cerr << "LZMA error: " << int(lzma_ret) << endl;
				return 1;
			}
			
			if(stream.avail_in) {
				cerr << "something bad happened: " << int(lzma_ret) << endl;
				return 1;
			}
			
		} else {
			
			ofs.write(chunk, length);
			
		}
		
	}
	
	if(block.compressed) {
		
		if(lzma_ret != LZMA_STREAM_END && block.storedSize) {
			cerr << "unexpected LZMA stream end";
		}
		
		lzma_end(&stream);
	} */
	
	
	return 0;
}

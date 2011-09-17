
#include <zlib.h>
#include <bzlib.h>
#include <lzma.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>

using std::cout;
using std::cerr;
using std::string;
using std::endl;
using std::setw;
using std::setfill;
using std::hex;
using std::dec;

struct _Progress {
	
	std::ostringstream & get() {
		return oss;
	}
	
	_Progress(bool _newline) : newline(_newline) { }
	
	~_Progress() {
		
		std::string str = oss.str();
		
		for(size_t i = 0; i < last; i++) {
			cout.put(0x08);
		}
		
		cout << str;
		
		if(str.length() < last) {
			for(size_t i = str.length(); i < last; i++) {
				cout.put(' ');
			}
		} else {
			last = str.length();
		}
		
		if(newline) {
			cout << endl;
			last = 0;
		}
		
	}
	
private:
	
	static size_t last;
	
	bool newline;
	std::ostringstream oss;
	
};

size_t _Progress::last = 0;

#define Progress _Progress(false).get()
#define Info _Progress(true).get()

typedef std::ios_base strm;

int main(int argc, const char * argv[]) {
	
	std::ifstream ifs(argv[1], strm::in | strm::binary | strm::ate);
	
	if(!ifs.is_open()) {
		cerr << "error opening file" << endl;
		return 1;
	}
	
	size_t size = ifs.tellg();
	if(!size) {
		cerr << "cannot read file" << endl;
		return 1;
	}
	
	char * data = new char[size];
	if(ifs.seekg(strm::beg).read(data, size).fail()) {
		cerr << "error reading file" << endl;
		return 1;
	}
	
	ifs.close();
	
	size_t bufsize = size * 2;
	char * buffer = new char[bufsize];
	
	std::ofstream ofs;
	
	size_t zcandidates = 0, zhits = 0;
	size_t bzcandidates = 0, bzhits = 0;
	size_t lzmacandidates = 0, lzmahits = 0;
	
	for(size_t offset = 0; offset < size - 5; offset++) {
		
		if(offset % 100000 == 0) {
			Progress << setfill(' ') << setw(10) << offset << " / " << setfill(' ') << setw(10) << size << " (" << (offset * 100 / size) << "%)";
		}
		
		char * block = data + offset;
		size_t blocksize = size - offset;
		
		// Try to interpret as ZLIB datastream.
		do {
			
			unsigned short cmf = (unsigned char)block[0];
			if((cmf & 0xf) != 8 || (cmf >> 4) > 7) {
				// invalid CMF byte
				break;
			}
			
			unsigned short flg = (unsigned char)block[1];
			if((cmf * 256 + flg) % 31) {
				// invalid CMF or FLAG byte
				break;
			}
			
			zcandidates++;
			
			z_stream stream;
			
			stream.zalloc = Z_NULL;
			stream.zfree = Z_NULL;
			stream.avail_in = blocksize;
			stream.next_in = (Bytef*)block;
			
			int ret = inflateInit(&stream);
			if(ret != Z_OK) {
				cout << endl;
				cerr << "error initializing zlib stream";
				return 1;
			}
			
			do {
				
				stream.avail_out = bufsize;
				stream.next_out = (Bytef*)buffer;
				
				ret = inflate(&stream, Z_SYNC_FLUSH);
				
				switch (ret) {
					
					case Z_OK:
					case Z_STREAM_END:
					case Z_BUF_ERROR: {
						
						size_t filesize = bufsize - stream.avail_out;
						Info << "found " << setfill(' ') << setw(10) << filesize << " bytes @ " << hex << setfill(' ') << setw(8) << offset;
						
						if(!ofs.is_open()) {
							zhits++;
							std::ostringstream filename;
					filename << "file@" << hex << setfill('0') << setw(8) << offset << ".zlib.txt";
							ofs.open(filename.str().c_str(), strm::trunc | strm::binary | strm::out);
						}
						
						ofs.write(buffer, filesize);
						
					}
					
				}
				
			} while(ret == Z_BUF_ERROR || ret == Z_OK);
			
			if(ret != Z_DATA_ERROR && ret != Z_STREAM_END && ret != Z_NEED_DICT) {
				Info << "zlib error: " << ret;
			}
			
			ofs.close();
			
			inflateEnd(&stream);
			
		} while(false);
		
		
		// Try to interpret as a bzip[2] datastream.
		do {
			
			if(block[0] != 'B' || block[1] != 'Z') {
				// Invalid magic number.
				break;
			}
			
			if(block[2] != 'h' && block[2] != '0') {
				// Unknown bzip version.
				break;
			}
			
			bzcandidates++;
			
		} while(false);
		
		/*
		while(true) {
			
			unsigned int filesize = bufsize;
			int ret = BZ2_bzBuffToBuffDecompress(buffer, &filesize, data + offset, size - offset, 0, 0);
			
			if(ret == BZ_OUTBUFF_FULL) {
				delete[] buffer;
				bufsize = 3 * bufsize / 4;
				buffer = new char[bufsize];
				continue;
			}
			
			if(ret == BZ_OK) {
				
				std::ostringstream filename;
				filename << "file@" << offset << ".bzip2.txt";
				
				std::ofstream ofs(filename.str().c_str(), strm::trunc | strm::binary | strm::out);
				
				ofs.write(buffer, filesize);
				
			}
			
			break;
			
		}*/
		
		/*
		while(true) {
			
			size_t in_pos = 0;
			size_t filesize = 0;
			uint64_t limit = 1024ul * 1024 * 1024 * 1042;
			lzma_ret ret = lzma_stream_buffer_decode(&limit, 0, NULL, reinterpret_cast<const uint8_t *>(data + offset), &in_pos, size - offset, reinterpret_cast<uint8_t *>(buffer), &filesize, bufsize);
			
			if(ret == LZMA_BUF_ERROR) {
				delete[] buffer;
				bufsize = 3 * bufsize / 4;
				buffer = new char[bufsize];
				continue;
			}
			
			if(ret == LZMA_OK) {
				
				std::ostringstream filename;
				filename << "file@" << offset << ".lzma.txt";
				
				std::ofstream ofs(filename.str().c_str(), strm::trunc | strm::binary | strm::out);
				
				ofs.write(buffer, filesize);
				
			}
			
			break;
			
		}*/
		
	}
	
	Info << "Done: zlib: " << zhits << " / " << zcandidates << "  bzip: " << bzhits << " / " << bzcandidates << "  lzma: " << lzmahits << " / " << lzmacandidates;
	
}

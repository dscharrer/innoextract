
#include "stream/file.hpp"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/make_shared.hpp>

#include "setup/FileLocationEntry.hpp"
#include "setup/Version.hpp"
#include "stream/checksum.hpp"

namespace io = boost::iostreams;

namespace stream {

namespace {

class inno_exe_decoder_4108 : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	inno_exe_decoder_4108() { close(0); }
	
	template<typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n);
	
	
	template<typename Source>
	void close(const Source &) {
		addr_bytes_left = 0, addr_offset = 5;
	}
	
	uint32_t addr;
	size_t addr_bytes_left;
	uint32_t addr_offset;
	
};

class inno_exe_decoder_5200 : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	explicit inno_exe_decoder_5200(bool _flip_high_byte)
		: flip_high_byte(_flip_high_byte) { close(0); }
	
	template<typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n);
	
	template<typename Source>
	void close(const Source &) {
		offset = 0, flush_bytes = 0;
	}
	
private:
	
	/*
	 * call_instruction_decoder_5200 has three states:
	 *
	 * "initial" (flush_bytes == 0)
	 *  - Read individual bytes and write them directly to output.
	 *  - If the byte could be the start of a CALL or JMP instruction that doesn't span blocks,
	 *    set addr_bytes_left to -4.
	 *
	 * "address" (flush_bytes < 0 && flush_bytes >= -4)
	 *  - Read all four address bytes into buffer, incrementing flush_bytes for each byte read.
	 *  - Once the last byte has been read, transform the address and set flush_bytes to 4.
	 *  - If an EOF is encountered before all four bytes have been read, set flush_bytes to
	 *    4 + flush_bytes.
	 *
	 * "flush" (flush_bytes > 0 && flush_bytes <= 4)
	 *  - Write the first flush_bytes bytes of buffer to output.
	 *  - If there is not enough output space, write as much as possible and move to rest to
	 *    the start of buffer.
	 */
	
	static const size_t block_size = 0x10000;
	const bool flip_high_byte;
	
	uint32_t offset; //! Total number of bytes read from the source.
	
	int8_t flush_bytes;
	uint8_t buffer[4];
	
};

// Implementation:

template<typename Source>
std::streamsize inno_exe_decoder_4108::read(Source & src, char * dest, std::streamsize n) {
	
	for(std::streamsize i = 0; i < n; i++, addr_offset++) {
		
		int byte = boost::iostreams::get(src);
		if(byte == EOF) { return i ? i : EOF; }
		if(byte == boost::iostreams::WOULD_BLOCK) { return i; }
		
		if(addr_bytes_left == 0) {
			
			// Check if this is a CALL or JMP instruction.
			if(byte == 0xe8 || byte == 0xe9) {
				addr = -addr_offset;
				addr_bytes_left = 4;
			}
			
		} else {
			addr += uint8_t(byte);
			byte = uint8_t(addr);
			addr >>= 8;
			addr_bytes_left--;
		}
		
		*dest++ = char(uint8_t(byte));
	}
	
	return n;
}

template<typename Source>
std::streamsize inno_exe_decoder_5200::read(Source & src, char * dest, std::streamsize n) {
	
	char * end = dest + n;
	
	//! Total number of filtered bytes read and written to dest.
#define total_read     (n - (end - dest))
	
#define flush(N) \
	{ \
		if((N) > 0) { \
			flush_bytes = (N); \
			size_t buffer_i = 0; \
			do { \
				if(dest == end) { \
					memmove(buffer, buffer + buffer_i, size_t(flush_bytes)); \
					return total_read; \
				} \
				*dest++ = char(buffer[buffer_i++]); \
			} while(--flush_bytes); \
		} \
	} (void)0
	
	
	// Flush already processed address bytes.
	flush(flush_bytes);
	
	while(dest != end) {
		
		if(!flush_bytes) {
			
			// Check if this is a CALL or JMP instruction.
			int byte = boost::iostreams::get(src);
			if(byte == EOF) { return total_read ? total_read : EOF; }
			if(byte == boost::iostreams::WOULD_BLOCK) { return total_read; }
			*dest++ = char(byte);
			offset++;
			if(byte != 0xe8 && byte != 0xe9) {
				// Not a CALL or JMP instruction.
				continue;
			}
			
			const size_t block_size_left = block_size - ((offset - 1) % block_size);
			if(block_size_left < 5) {
				// Ignore instructions that span blocks.
				continue;
			}
			
			flush_bytes = -4;
		}
		
		assert(flush_bytes < 0);
		
		// Read all four address bytes.
		char * dst = reinterpret_cast<char *>(buffer + 4 + flush_bytes);
		std::streamsize nread = boost::iostreams::read(src, dst, -flush_bytes);
		if(nread == EOF) {
			flush(int8_t(4 + flush_bytes));
			return total_read ? total_read : EOF;
		}
		flush_bytes = int8_t(flush_bytes + nread), offset += uint32_t(nread);
		if(flush_bytes) { return total_read; }
		
		// Verify that the high byte of the address is 0x00 or 00xff.
		if(buffer[3] == 0x00 || buffer[3] == 0xff) {
			
			uint32_t addr = offset & 0xffffff; // may wrap, but OK
			
			uint32_t rel = buffer[0] | (uint32_t(buffer[1]) << 8) | (uint32_t(buffer[2]) << 16);
			rel -= addr;
			buffer[0] = uint8_t(rel), buffer[1] = uint8_t(rel >> 8), buffer[2] = uint8_t(rel >> 16);
			
			if(flip_high_byte) {
				// For a slightly higher compression ratio, we want the resulting high
				// byte to be 0x00 for both forward and backward jumps. The high byte
				// of the original relative address is likely to be the sign extension
				// of bit 23, so if bit 23 is set, toggle all bits in the high byte.
				if(rel & 0x800000) {
					buffer[3] = uint8_t(~buffer[3]);
				}
			}
			
		} else {
			// This is most likely not a CALL or JUMP.
		}
		
		flush(4);
	}
	
	return total_read;
	
#undef flush
#undef total_read
	
}

} // anonymous namespace

file_reader::pointer file_reader::get(base_type & base, const FileLocationEntry & location,
                                      const InnoVersion & version, crypto::checksum * checksum) {
	
	boost::shared_ptr<io::filtering_istream> result = boost::make_shared<io::filtering_istream>();
	
	result->push(stream::checksum_filter(checksum, location.checksum.type), 8192);
	
	if(location.options & FileLocationEntry::CallInstructionOptimized) {
		if(version < INNO_VERSION(5, 2, 0)) {
			result->push(inno_exe_decoder_4108(), 8192);
		} else {
			result->push(inno_exe_decoder_5200(version >= INNO_VERSION(5, 3, 9)), 8192);
		}
	}
	
	result->push(io::restrict(base, 0, int64_t(location.file_size)));
	
	return result;
}

} // namespace stream

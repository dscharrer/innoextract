/*
 * Copyright (C) 2011-2019 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*!
 * \file
 *
 * Filters to be used with boost::iostreams for undoing transformations Inno Setup applies
 * to stored executable files to make them more compressible.
 */
#ifndef INNOEXTRACT_STREAM_EXEFILTER_HPP
#define INNOEXTRACT_STREAM_EXEFILTER_HPP

#include <stddef.h>
#include <iosfwd>
#include <cassert>

#include <boost/cstdint.hpp>
#include <boost/iostreams/char_traits.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/get.hpp>
#include <boost/iostreams/read.hpp>

namespace stream {

/*!
 * Filter to decode executable files stored by Inno Setup versions before 5.2.0.
 *
 * Essentially, it tries to change the addresses stored for x86 CALL and JMP instructions
 * to be relative to the instruction's position.
 */
class inno_exe_decoder_4108 : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	inno_exe_decoder_4108() { close(0); }
	
	template <typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n);
	
	
	template <typename Source>
	void close(const Source & /* source */) {
		addr = 0, addr_bytes_left = 0, addr_offset = 5;
	}
	
private:
	
	boost::uint32_t addr;
	size_t addr_bytes_left;
	boost::uint32_t addr_offset;
	
};

/*!
 * Filter to decode executable files stored by Inno Setup versions after 5.2.0.
 *
 * It tries to change the addresses stored for x86 CALL and JMP instructions to be
 * relative to the instruction's position, plus a few other tweaks.
 */
class inno_exe_decoder_5200 : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	/*!
	 * \param flip_high_bytes true if the high byte of addresses is flipped if bit 23 is set.
	 *                        This optimization is used in Inno Setup 5.3.9 and later.
	 */
	explicit inno_exe_decoder_5200(bool flip_high_bytes)
		: flip_high_byte(flip_high_bytes) { close(0); }
	
	template <typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n);
	
	template <typename Source>
	void close(const Source & /* source */) {
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
	
	boost::uint32_t offset; //! Total number of bytes read from the source.
	
	boost::int8_t flush_bytes;
	boost::uint8_t buffer[4];
	
};

// Implementation:

template <typename Source>
std::streamsize inno_exe_decoder_4108::read(Source & src, char * dest, std::streamsize n) {
	
	for(std::streamsize i = 0; i < n; i++, addr_offset++) {
		
		int byte = boost::iostreams::get(src);
		if(byte == EOF) { return i ? i : EOF; }
		if(byte == boost::iostreams::WOULD_BLOCK) { return i; }
		
		if(addr_bytes_left == 0) {
			
			// Check if this is a CALL or JMP instruction.
			if(byte == 0xe8 || byte == 0xe9) {
				addr = ~addr_offset + 1;
				addr_bytes_left = 4;
			}
			
		} else {
			addr += boost::uint8_t(byte);
			byte = boost::uint8_t(addr);
			addr >>= 8;
			addr_bytes_left--;
		}
		
		*dest++ = char(boost::uint8_t(byte));
	}
	
	return n;
}

template <typename Source>
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
			flush(boost::int8_t(4 + flush_bytes));
			return total_read ? total_read : EOF;
		}
		flush_bytes = boost::int8_t(flush_bytes + nread), offset += boost::uint32_t(nread);
		if(flush_bytes) { return total_read; }
		
		// Verify that the high byte of the address is 0x00 or 00xff.
		if(buffer[3] == 0x00 || buffer[3] == 0xff) {
			
			boost::uint32_t addr = offset & 0xffffff; // may wrap, but OK
			
			boost::uint32_t rel = buffer[0] | (boost::uint32_t(buffer[1]) << 8)
			                                | (boost::uint32_t(buffer[2]) << 16);
			rel -= addr;
			buffer[0] = boost::uint8_t(rel);
			buffer[1] = boost::uint8_t(rel >> 8);
			buffer[2] = boost::uint8_t(rel >> 16);
			
			if(flip_high_byte) {
				// For a slightly higher compression ratio, we want the resulting high
				// byte to be 0x00 for both forward and backward jumps. The high byte
				// of the original relative address is likely to be the sign extension
				// of bit 23, so if bit 23 is set, toggle all bits in the high byte.
				if(rel & 0x800000) {
					buffer[3] = boost::uint8_t(~buffer[3]);
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

} // namespace stream

#endif // INNOEXTRACT_STREAM_EXEFILTER_HPP

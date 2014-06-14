/*
 * Copyright (C) 2014 Daniel Scharrer
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
 * Parser for ANSI escape code sequences
 */
#ifndef INNOEXTRACT_UTIL_ANSI_HPP
#define INNOEXTRACT_UTIL_ANSI_HPP

#include <stddef.h>

#include <algorithm>
#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>

#include <boost/iostreams/categories.hpp>

#include "util/load.hpp"
#include "util/types.hpp"

namespace util {

/*!
 * \brief CRTP base class that parses ANSI escape sequences
 *
 * Currently only supports parsing CSI sequences.
 *
 * This class cannot be instantiated by itself. Instead create a derivec class
 * \code class derived : public util::ansi_console_parser<derived>; \endcode
 * that re-implements \ref handle_command() and \ref handle_text().
 */
template <typename Impl>
class ansi_console_parser : public util::static_polymorphic<Impl> {
	
	//! Character that started the current control sequence, or \c 0
	char in_command;
	
	//! Buffer for control sequences if they span more than one flush
	std::vector<char> command;
	
protected:
	
	enum {
		ESC = '\x1b',
		CSI = '[', //!< Control Sequence Indicator (preceded by \ref ESC)
		UTF8CSI0 = '\xc2', //!< UTF-8 Control Sequence Indicator, first byte
		UTF8CSI1 = '\x9b', //!< UTF-8 Control Sequence Indicator, second byte
		Separator = ';' //! Separator for codes in CSI control sequences
	};
	
	enum CommandType {
		CUU = 'A', //!< Cursor Up
		CUD = 'B', //!< Cursor Down
		CUF = 'C', //!< Cursor Forward
		CUB = 'D', //!< Cursor Back
		CNL = 'E', //!< Cursor Next Line
		CPL = 'F', //!< Cursor Previous Line
		CHA = 'G', //!< Cursor Horizontal Absolute
		CUP = 'H', //!< Cursor Position
		ED  = 'J', //!< Erase Display
		EL  = 'K', //!< Erase in Line
		SU  = 'S', //!< Scroll Up
		SD  = 'T', //!< Scroll Down
		HVP = 'f', //!< Horizontal and Vertical Position
		SGR = 'm', //!< Select Graphic Rendition
		DSR = 'n', //!< Device Status Report
		SCP = 's', //!< Save Cursor Position
		RCP = 'u', //!< Restore Cursor Position
	};
	
private:
	
	struct is_start_char {
		bool operator()(char c) {
			return (c == ESC /* escape */ || c == UTF8CSI0 /* first byte of UTF-8 CSI */);
		}
	};
	
	struct is_end_char {
		bool operator()(char c) {
			return (c >= 64 && c < 127);
		}
	};
	
protected:
	
	#ifdef DEBUG
	void error(const std::string & str) {
		this->impl().handle_text(str.data(), str.length());
	}
	#endif
	
	/*!
	 * \brief Read one code form a command sequence
	 *
	 * \param s   Current position in the command sequence.
	 * \param end End of the command sequence.
	 *
	 * Each command sequence contains contains at least one code. Once there are no more
	 * commands in the command sequence, \c s will be set to \c NULL. After that has
	 * happened \ref read_code() should must not be called with the (\c s, \c end) pair.
	 *
	 * The meaning of th returned code depends on the type of the command sequence.
	 *
	 * \return the next code in the command sequence or unsigned(-1) if there was an error.
	 */
	unsigned read_code(const char * & s, const char * end) {
		
		const char * sep = std::find(s, end, Separator);
		
		unsigned code = unsigned(-1);
		try {
			code = (s == sep) ? 0u : util::to_unsigned(s, size_t(sep - s));
		} catch(...) {
			#ifdef DEBUG
			std::ostringstream oss;
			oss << "(bad command code: \"";
			oss.write(s, sep - s);
			oss << "\")";
			error(oss.str());
			#endif
		}
		
		if(sep == end) {
			s = NULL;
		} else {
			s = sep + 1;
		}
		
		return code;
	}
	
private:
	
	const char * read_command(const char * s, const char * end) {
		
		if(s == end) {
			return end; // Need to be able to read something
		}
		
		if(command.empty() && *s != (in_command == ESC ? CSI : UTF8CSI1)) {
			switch(in_command) {
				case ESC: /* escaped char */ break;
				default: {
					char utf8[] = { in_command, *s };
					this->impl().handle_text(utf8, 2);
				}
			}
			return s + 1; // Not a Control Sequence Initiator
		}
		
		const char * cmd = std::find_if(command.empty() ? s + 1 : s, end, is_end_char());
		
		const char * cs = s;
		const char * ce = cmd;
		
		if(!command.empty() || cmd == end) {
			command.insert(command.end(), s, cmd);
			cs = &command.front();
			ce = cs + command.size();
		}
		
		if(cmd == end) {
			return end; // Command not over yet
		}
		
		// Extract the command type
		CommandType type = CommandType(*cmd);
		
		// Skip starting character (part of the CSI sequence)
		cs++;
		
		this->impl().handle_command(type, cs, ce);
		
		in_command = 0;
		command.clear();
		
		return cmd + 1;
	}
	
public:
	
	ansi_console_parser() : in_command(0) { }
	
	typedef char char_type;
	typedef boost::iostreams::sink_tag category;
	
	/*!
	 * \brief Will be called when an ANSI escape sequence has been found
	 *
	 * Derived classes must override this.
	 *
	 * \param type  The type of command. This is the last character of the escape sequence.
	 * \param codes Start of the code sequence. Use \ref read_code() to read codes.
	 * \param end   End of the code sequence.
	 */
	void handle_command(CommandType type, const char * codes, const char * end);
	
	/*!
	 * \brief Will be called when plain text has been found
	 *
	 * Derived classes must override this.
	 *
	 * \param s Pointer to the text.
	 * \param n length of the text in bytes.
	 */
	void handle_text(const char * s, size_t n);
	
	/*!
	 * \brief Parse \c n characters from \c s
	 *
	 * The string may contain multiple escape sequences and escape sequences may span
	 * multiple calls to write().
	 *
	 * All escape sequences are passed to \ref handle_command() while plain text segments
	 * are passed to \ref handle_text().
	 *
	 * \return n
	 */
	std::streamsize write(const char * s, std::streamsize n) {
		
		const char * begin = s;
		const char * end = s + n;
		
		if(in_command) {
			s = read_command(s, end);
		}
		
		while(s != end) {
			
			const char * cmd = std::find_if(s, end, is_start_char());
			
			// Output the non-escaped text
			this->impl().handle_text(s, size_t(cmd - s));
			
			if(cmd == end) {
				s = end;
				break;
			}
			
			// A command possibly starts here
			in_command = *cmd;
			s = read_command(cmd + 1, end);
			
		}
		
		return s - begin;
	}
	
};

} // namespace util

#endif // INNOEXTRACT_UTIL_ANSI_HPP

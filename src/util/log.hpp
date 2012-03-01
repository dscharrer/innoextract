/*
 * Copyright (C) 2011 Daniel Scharrer
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

#ifndef INNOEXTRACT_UTIL_LOG_HPP
#define INNOEXTRACT_UTIL_LOG_HPP

#include <sstream>
#include <string>

#ifdef DEBUG
#define debug(...)    \
	if(::logger::debug) \
		::logger(__FILE__, __LINE__, ::logger::Debug) << __VA_ARGS__
#define log_info    ::logger(__FILE__, __LINE__, ::logger::Info)
#define log_warning ::logger(__FILE__, __LINE__, ::logger::Warning)
#define log_error   ::logger(__FILE__, __LINE__, ::logger::Error)
#else
#define debug(...)
#define log_info    ::logger(::logger::Info)
#define log_warning ::logger(::logger::Warning)
#define log_error   ::logger(::logger::Error)
#endif

/*!
 * logger class that allows longging via the stream operator.
 */
class logger {
	
public:
	
	enum log_level {
		Debug,
		Info,
		Warning,
		Error
	};
	
private:
	
#ifdef DEBUG
	const char * const file;
	const int line;
#endif
	
	const log_level level;
	
	std::ostringstream buffer; //! Buffer for the log message excluding level, file and line.
	
public:
	
	static size_t total_warnings;
	static size_t total_errors;
	
	static bool debug;
	
#ifdef DEBUG
	inline logger(const char * _file, int _line, log_level _level)
		: file(_file), line(_line), level(_level) { }
#else
	inline logger(log_level _level) : level(_level) { }
#endif
	
	template<class T>
	inline logger & operator<<(const T & i) {
		buffer << i;
		return *this;
	}
	
	~logger();
	
};

#endif // INNOEXTRACT_UTIL_LOG_HPP

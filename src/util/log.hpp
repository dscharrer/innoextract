/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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
 * Logging functions.
 */
#ifndef INNOEXTRACT_UTIL_LOG_HPP
#define INNOEXTRACT_UTIL_LOG_HPP

#include <sstream>
#include <string>

#ifdef DEBUG
#define debug(...) \
	if(::logger::debug) \
		::logger(::logger::Debug) << __VA_ARGS__
#else
#define debug(...)
#endif

#define log_info \
	if(!::logger::quiet) \
		::logger(::logger::Info)
#define log_warning ::logger(::logger::Warning)
#define log_error   ::logger(::logger::Error)

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
	
	const log_level level;
	
	std::ostringstream buffer; //! Buffer for the log message excluding level, file and line.
	
public:
	
	static size_t total_warnings; //! Total number of \ref log_warning uses so far.
	static size_t total_errors;   //! Total number of \ref log_error uses so far.
	
	static bool debug; //! Is \ref debug output enabled?
	static bool quiet; //! Is \ref log_info disabled?
	
	/*!
	 * Construct a log line output stream.
	 *
	 * You probably don't want to use this directly - use \ref debug, \ref log_info,
	 * \ref log_warning and \ref log_error instead.
	 */
	explicit logger(log_level _level) : level(_level) { }
	
	template<class T>
	logger & operator<<(const T & i) {
		buffer << i;
		return *this;
	}
	
	~logger();
	
};

#endif // INNOEXTRACT_UTIL_LOG_HPP


#ifndef INNOEXTRACT_UTIL_LOG_HPP
#define INNOEXTRACT_UTIL_LOG_HPP

#include <sstream>
#include <string>

#ifdef _DEBUG
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
	
#ifdef _DEBUG
	const char * const file;
	const int line;
#endif
	
	const log_level level;
	
	std::ostringstream buffer; //! Buffer for the log message excluding level, file and line.
	
public:
	
	static size_t total_warnings;
	static size_t total_errors;
	
	static bool debug;
	
#ifdef _DEBUG
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

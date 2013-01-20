/*
 * Copyright (C) 2013 Daniel Scharrer
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

#include "util/time.hpp"

#include "configure.hpp"

#if INNOEXTRACT_HAVE_TIMEGM || INNOEXTRACT_HAVE_MKGMTIME \
    || INNOEXTRACT_HAVE_GMTIME_R || INNOEXTRACT_HAVE_GMTIME_S
#include <time.h>
#endif

#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#if INNOEXTRACT_HAVE_UTIMENSAT && INNOEXTRACT_HAVE_AT_FDCWD
#include <sys/stat.h>
#include <fcntl.h>
#elif !defined(_WIN32) && INNOEXTRACT_HAVE_UTIMES
#include <sys/time.h>
#elif !defined(_WIN32)
#include <boost/filesystem/operations.hpp>
#endif

namespace util {

static void set_timezone(const char * value) {
	
	const char * variable = "TZ";
	
#if defined(WIN32)
	
	SetEnvironmentVariable(variable, value);
	_tzset();
	
#else
	
	if(value) {
		setenv(variable, value, 1);
	} else {
		unsetenv(variable);
	}
	tzset();
	
#endif
	
}

std::time_t parse_time(std::tm tm) {
	
	tm.tm_isdst = 0;
	
#if INNOEXTRACT_HAVE_TIMEGM
	
	// GNU / BSD extension
	
	return timegm(&tm);
	
#elif INNOEXTRACT_HAVE_MKGMTIME
	
	// Windows
	
	return _mkgmtime(&tm);
	
#else
	
	// Standard, but not thread-safe - should be OK for our use though
	
	char * tz = getenv("TZ");
	
	set_timezone("UTC");
	
	std::time_t ret = std::mktime(&tm);
	
	set_timezone(tz);
	
	return ret;
	
#endif
	
}

std::tm format_time(time_t t) {
	
	std::tm ret;
	
#if INNOEXTRACT_HAVE_GMTIME_R
	
	// POSIX.1
	
	gmtime_r(&t, &ret);
	
#elif INNOEXTRACT_HAVE_GMTIME_S
	
	// Windows (MSVC)
	
	_gmtime_s(&ret, &t);
	
#else
	
	// Standard C++, but may not be thread-safe
	
	std::tm * tmp = std::gmtime(&t);
	if(tmp) {
		ret = *tmp;
	} else {
		ret.tm_year = ret.tm_mon = ret.tm_mday = -1;
		ret.tm_hour = ret.tm_min = ret.tm_sec = -1;
		ret.tm_isdst = -1;
	}
	
#endif
	
	return ret;
}

std::time_t to_local_time(std::time_t t) {
	
	// Format time as UTC ...
	std::tm time = format_time(t);
	
	// ... and interpret it as local time
	time.tm_isdst = 0;
	return std::mktime(&time);
}

void set_local_timezone(std::string timezone) {
	
	/*
	 * The TZ variable interprets the offset as the change from local time 
	 * to UTC while everyone else does the opposite.
	 * We flip the direction so that timezone strings such as GMT+1 work as expected.
	 */
	for(size_t i = 0; i < timezone.length(); i++) {
		if(timezone[i] == '+') {
			timezone[i] = '-';
		} else if(timezone[i] == '-') {
			timezone[i] = '+';
		}
	}
	
	set_timezone(timezone.c_str());
}

#if defined(_WIN32)

static HANDLE open_file(LPCSTR name) {
	return CreateFileA(name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
}

static HANDLE open_file(LPCWSTR name) {
	return CreateFileW(name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
}

#endif

bool set_file_time(const boost::filesystem::path & path, std::time_t t, uint32_t nsec) {
	
#if INNOEXTRACT_HAVE_UTIMENSAT && INNOEXTRACT_HAVE_AT_FDCWD
	
	// nanosecond precision, for Linux and POSIX.1-2008+ systems
	
	struct timespec times[2];
	times[0].tv_sec = t;
	times[0].tv_nsec = int32_t(nsec);
	times[1] = times[0];
	
	return (utimensat(AT_FDCWD, path.string().c_str(), times, 0) == 0);
	
#elif defined(_WIN32)
	
	// 100-nanosecond precision, for Windows
	
	// Prevent unused function warnings
	(void)(HANDLE(*)(LPCSTR))open_file;
	(void)(HANDLE(*)(LPCWSTR))open_file;
	
	HANDLE handle = open_file(path.c_str());
	if(handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	
	// Convert the std::time_t and nanoseconds to a FILETIME struct
	static const int64_t FiletimeOffset = 0x19DB1DED53E8000ll;
	int64_t time = int64_t(t) * 10000000 + int64_t(nsec) / 100;
	time += FiletimeOffset;
	FILETIME filetime;
	filetime.dwLowDateTime = DWORD(time);
	filetime.dwHighDateTime = DWORD(time >> 32);
	
	bool ret = SetFileTime(handle, &filetime, &filetime, &filetime);
	CloseHandle(handle);
	
	return ret;
	
#elif INNOEXTRACT_HAVE_UTIMES
	
	// microsecond precision, for older POSIX systems (4.3BSD, POSIX.1-2001)
	
	struct timeval times[2];
	times[0].tv_sec = t;
	times[0].tv_usec = int32_t(nsec / 1000);
	times[1] = times[0];
	
	return (utimes(path.string().c_str(), times) == 0);
	
#else
	
	// fallback with second precision or worse
	
	try {
		(void)nsec; // sub-second precision not supported by boost
		boost::filesystem::last_write_time(path, t);
		return true;
	} catch(...) {
		return false;
	}
	
#endif
	
}

} // namespace util

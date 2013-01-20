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

#include "util/time.hpp"

#include "configure.hpp"

#if INNOEXTRACT_HAVE_TIMEGM || INNOEXTRACT_HAVE_MKGMTIME \
    || INNOEXTRACT_HAVE_GMTIME_R || INNOEXTRACT_HAVE_GMTIME_S
#include <time.h>
#endif

#if !INNOEXTRACT_HAVE_TIMEGM
#include <stdlib.h>
#endif

#if INNOEXTRACT_HAVE_UTIMES
#include <sys/time.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <boost/filesystem/operations.hpp>
#endif

namespace util {

std::time_t parse_time(std::tm tm) {
	
#if INNOEXTRACT_HAVE_TIMEGM
	
	return timegm(&tm);
	
#elif INNOEXTRACT_HAVE_MKGMTIME
	
	return _mkgmtime(&tm);
	
#else
	
	char * tz = getenv("TZ");
	setenv("TZ", "UTC", 1);
	tzset();
	
	std::time_t ret = std::mktime(&tm);
	
	if(tz) {
		setenv("TZ", tz, 1);
	} else {
		unsetenv("TZ");
	}
	tzset();
	
	return ret;
	
#endif
	
}

std::tm format_time(time_t t) {
	
	std::tm ret;
	
#if INNOEXTRACT_HAVE_GMTIME_R
	
	gmtime_r(&t, &ret);
	
#elif INNOEXTRACT_HAVE_GMTIME_S
	
	_gmtime_s(&ret, &t);
	
#else
	
	// Hope that this is threadsafe...
	ret = *gmtime(&t);
	
#endif
	
	return ret;
}

time_t to_local_time(time_t t) {
	
	// Format time as UTC ...
	std::tm time = format_time(t);
	
	//... and interpret it as local time
	return std::mktime(&time);
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
	
#if INNOEXTRACT_HAVE_UTIMES
	
	struct timeval times[2];
	
	times[0].tv_sec = t;
	times[0].tv_usec = long(nsec / 1000);
	times[1] = times[0];
	
	return (utimes(path.c_str(), times) == 0);
	
#elif defined(_WIN32)
	
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
	
#else
	
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

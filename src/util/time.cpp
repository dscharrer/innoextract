/*
 * Copyright (C) 2013-2019 Daniel Scharrer
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

#if INNOEXTRACT_HAVE_TIMEGM || INNOEXTRACT_HAVE_GMTIME_R || defined(_WIN32)
#include <time.h>
#endif

#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#if INNOEXTRACT_HAVE_DLSYM
#include <dlfcn.h>
#endif

#if INNOEXTRACT_HAVE_AT_FDCWD
#include <fcntl.h>
#endif

#if INNOEXTRACT_HAVE_UTIMENSAT && INNOEXTRACT_HAVE_AT_FDCWD
#include <sys/stat.h>
#elif !defined(_WIN32) && INNOEXTRACT_HAVE_UTIMES
#include <sys/time.h>
#elif !defined(_WIN32)
#include <boost/filesystem/operations.hpp>
#endif

#include "util/log.hpp"

namespace util {

#if defined(_WIN32)

static const boost::int64_t FiletimeOffset = 0x19DB1DED53E8000ll;

static time from_filetime(FILETIME ft) {
	
	boost::int64_t filetime = boost::int64_t(ft.dwHighDateTime) << 32;
	filetime += boost::int64_t(ft.dwLowDateTime);
	
	filetime -= FiletimeOffset;
	
	return filetime / 10000000;
}

static FILETIME to_filetime(time t, boost::uint32_t nsec = 0) {
	
	boost::int64_t filetime64 = boost::int64_t(t) * 10000000 + boost::int64_t(nsec) / 100 + FiletimeOffset;
	
	FILETIME filetime;
	filetime.dwLowDateTime = DWORD(filetime64);
	filetime.dwHighDateTime = DWORD(filetime64 >> 32);
	return filetime;
}

#endif

static void set_timezone(const char * value) {
	
	const char * variable = "TZ";
	
	#if defined(_WIN32)
	
	SetEnvironmentVariableA(variable, value);
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

time parse_time(std::tm tm) {
	
	tm.tm_isdst = 0;
	
	#if defined(_WIN32)
	
	// Windows
	
	SYSTEMTIME st;
	st.wYear         = WORD(tm.tm_year + 1900);
	st.wMonth        = WORD(tm.tm_mon + 1);
	st.wDay          = WORD(tm.tm_mday);
	st.wHour         = WORD(tm.tm_hour);
	st.wMinute       = WORD(tm.tm_min);
	st.wSecond       = WORD(tm.tm_sec);
	st.wMilliseconds = 0;
	
	FILETIME ft;
	if(!SystemTimeToFileTime(&st, &ft)) {
		return 0;
	}
	return from_filetime(ft);
	
	#elif INNOEXTRACT_HAVE_TIMEGM
	
	// GNU / BSD extension
	
	return timegm(&tm);
	
	#else
	
	// Standard, but not thread-safe - should be OK for our use though
	
	char * tz = getenv("TZ");
	
	set_timezone("UTC");
	
	time ret = std::mktime(&tm);
	
	set_timezone(tz);
	
	return ret;
	
	#endif
	
}

template <typename Time>
static Time to_time_t(time t, const char * file = "conversion") {
	
	Time ret = Time(t);
	
	if(time(ret) != t) {
		log_warning << "Truncating timestamp " << t << " to " << ret << " for " << file;
	}
	
	return ret;
}

std::tm format_time(time t) {
	
	std::tm ret;
	
	#if defined(_WIN32)
	
	// Windows
	
	FILETIME ft = to_filetime(t);
	
	SYSTEMTIME st;
	if(FileTimeToSystemTime(&ft, &st)) {
		ret.tm_year = int(st.wYear) - 1900;
		ret.tm_mon  = int(st.wMonth) - 1;
		ret.tm_wday = int(st.wDayOfWeek);
		ret.tm_mday = int(st.wDay);
		ret.tm_hour = int(st.wHour);
		ret.tm_min  = int(st.wMinute);
		ret.tm_sec  = int(st.wSecond);
	} else {
		ret.tm_year = ret.tm_mon = ret.tm_mday = -1;
		ret.tm_hour = ret.tm_min = ret.tm_sec = -1;
	}
	ret.tm_isdst = -1;
	
	#elif INNOEXTRACT_HAVE_GMTIME_R
	
	// POSIX.1
	
	time_t tt = to_time_t<time_t>(t);
	gmtime_r(&tt, &ret);
	
	#else
	
	// Standard C++
	
	std::time_t tt = to_time_t<std::time_t>(t);
	std::tm * tmp = std::gmtime(&tt); /* not thread-safe */
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

time to_local_time(time t) {
	
	// Format time as UTC ...
	std::tm datetime = format_time(t);
	
	// ... and interpret it as local time
	datetime.tm_isdst = 0;
	#if defined(_WIN32)
	return _mktime64(&datetime);
	#else
	return std::mktime(&datetime);
	#endif
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

#if INNOEXTRACT_HAVE_DYNAMIC_UTIMENSAT
extern "C" typedef int (*utimensat_proc)
	(int fd, const char *path, const struct timespec times[2], int flag);
#endif

bool set_file_time(const boost::filesystem::path & path, time sec, boost::uint32_t nsec) {
	
	#if (INNOEXTRACT_HAVE_DYNAMIC_UTIMENSAT || INNOEXTRACT_HAVE_UTIMENSAT) \
	    && INNOEXTRACT_HAVE_AT_FDCWD
	
	// nanosecond precision, for Linux and POSIX.1-2008+ systems
	
	struct timespec timens[2];
	timens[0].tv_sec = to_time_t<time_t>(sec, path.string().c_str());
	timens[0].tv_nsec = boost::int32_t(nsec);
	timens[1] = timens[0];
	
	#endif
	
	#if INNOEXTRACT_HAVE_DYNAMIC_UTIMENSAT && INNOEXTRACT_HAVE_AT_FDCWD
	
	static utimensat_proc utimensat_func = reinterpret_cast<utimensat_proc>(dlsym(RTLD_DEFAULT, "utimensat"));
	if(utimensat_func) {
		return (utimensat_func(AT_FDCWD, path.string().c_str(), timens, 0) == 0);
	}
	
	#endif
	
	#if INNOEXTRACT_HAVE_UTIMENSAT && INNOEXTRACT_HAVE_AT_FDCWD
	
	return (utimensat(AT_FDCWD, path.string().c_str(), timens, 0) == 0);
	
	#elif defined(_WIN32)
	
	// 100-nanosecond precision, for Windows
	
	// Prevent unused function warnings
	(void)static_cast<HANDLE(*)(LPCSTR)>(open_file);
	(void)static_cast<HANDLE(*)(LPCWSTR)>(open_file);
	
	HANDLE handle = open_file(path.c_str());
	if(handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	
	FILETIME filetime = to_filetime(sec, nsec);
	
	bool ret = (SetFileTime(handle, &filetime, &filetime, &filetime) != 0);
	CloseHandle(handle);
	
	return ret;
	
	#elif INNOEXTRACT_HAVE_UTIMES
	
	// microsecond precision, for older POSIX systems (4.3BSD, POSIX.1-2001)
	
	struct timeval times[2];
	times[0].tv_sec = to_time_t<time_t>(sec, path.string().c_str());
	times[0].tv_usec = boost::int32_t(nsec / 1000);
	times[1] = times[0];
	
	return (utimes(path.string().c_str(), times) == 0);
	
	#else
	
	// fallback with second precision or worse
	
	try {
		(void)nsec; // sub-second precision not supported by Boost
		std::time_t tt = to_time_t<std::time_t>(sec, path.string().c_str());
		boost::filesystem::last_write_time(path, tt);
		return true;
	} catch(...) {
		return false;
	}
	
	#endif
	
}

} // namespace util

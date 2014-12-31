/*
 * Copyright (C) 2013-2014 Daniel Scharrer
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

#include "util/process.hpp"

#include <sstream>
#include <iostream>

#include "configure.hpp"

#if defined(_WIN32)

#include <string.h>
#include <windows.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#elif INNOEXTRACT_HAVE_POSIX_SPAWNP || (INNOEXTRACT_HAVE_FORK && INNOEXTRACT_HAVE_EXECVP)

#if INNOEXTRACT_HAVE_POSIX_SPAWNP
#include <spawn.h>
#if defined(__FreeBSD__) && defined(__GNUC__) && __GNUC__ >= 4
/*
 * When combining -flto and -fvisibility=hidden we and up with a hidden
 * 'environ' symbol in crt1.o on FreeBSD 9, which causes the link to fail.
 */
extern char ** environ __attribute__((visibility("default")));
#else
extern char ** environ;
#endif
#else
#include <unistd.h>
#endif

#if INNOEXTRACT_HAVE_WAITPID
#include <sys/wait.h>
#endif

#else

#include <cstdlib>

#endif

namespace util {

#if defined(_WIN32) || !(INNOEXTRACT_HAVE_POSIX_SPAWNP \
                         || (INNOEXTRACT_HAVE_FORK && INNOEXTRACT_HAVE_EXECVP))
static std::string format_command_line(const char * const args[]) {
	
	std::ostringstream oss;
	
	for(size_t i = 0; args[i]; i++) {
		if(i != 0) {
			oss << ' ';
		}
		oss << '"';
		for(const char * arg = args[i]; *arg; arg++) {
			char c = *arg;
			if(c == '\\' || c == '\"' || c == ' ' || c == '\'' || c == '$' || c == '!') {
				oss << '\\';
			}
			oss << c;
		}
		oss << '"';
	}
	
	return oss.str();
}
#endif

#if defined(_WIN32)
static WCHAR * utf8_to_wchar(const char * string) {
	int n = MultiByteToWideChar(CP_UTF8, 0, string, -1, NULL, 0);
	WCHAR * wstr = new WCHAR[n];
	MultiByteToWideChar(CP_UTF8, 0, string, -1, wstr, n);
	return wstr;
}
#endif

int run(const char * const args[]) {
	
	std::cout.flush();
	std::cerr.flush();
	
#if defined(_WIN32)
	
	// Format the command line arguments
	WCHAR * exe = utf8_to_wchar(args[0]);
	WCHAR * cmdline = utf8_to_wchar(format_command_line(args + 1).c_str());
	
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	
	bool success = (CreateProcessW(exe, cmdline, 0, 0, 0, 0, 0, 0, &si, &pi) != 0);
	
	delete[] cmdline;
	delete[] exe;
	
	if(!success) {
		return -1; // Could not start process
	}
	
	int status = int(WaitForSingleObject(pi.hProcess, INFINITE));
	
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	return status;
	
#elif INNOEXTRACT_HAVE_POSIX_SPAWNP || (INNOEXTRACT_HAVE_FORK && INNOEXTRACT_HAVE_EXECVP)
	
	char ** argv = const_cast<char **>(args);
	
	pid_t pid = -1;
	
	#if INNOEXTRACT_HAVE_POSIX_SPAWNP
	
	// Fast POSIX implementation: posix_spawnp avoids unnecessary vm copies
	
	// Run the executable in a new process
	(void)posix_spawnp(&pid, argv[0], NULL, NULL, argv, environ);
	
	#else
	
	// Compatibility POSIX implementation
	
	// Start a new process
	pid = fork();
	if(pid == 0) {
		
		// Run the executable
		(void)execvp(argv[0], argv);
		
		exit(-1);
	}
	
	#endif
	
	if(pid < 0) {
		return -1;
	}
	
	#if INNOEXTRACT_HAVE_WAITPID
	int status;
	(void)waitpid(pid, &status, 0);
	if(WIFEXITED(status) && (WEXITSTATUS(status) >= 0 && WEXITSTATUS(status) < 127)) {
		return WEXITSTATUS(status);
	} else if(WIFSIGNALED(status)) {
		return -WTERMSIG(status);
	} else {
		return -1;
	}
	#else
	# warning "Waiting for processes not supported on this system."
	#endif
	
	return 0;
	
#else
	return std::system(format_command_line(args).c_str());
#endif
	
}

} // namespace util

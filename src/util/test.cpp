/*
 * Copyright (C) 2024 Daniel Scharrer
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

#include "util/test.hpp"

#include "util/windows.hpp"

#include "configure.hpp"

#if INNOEXTRACT_HAVE_ISATTY
#include <unistd.h>
#endif

namespace {

bool test_verbose = false;
bool test_progress = false;
int test_failed = 0;

} // anonymous namewspace

Testsuite * Testsuite::tests = NULL;

Testsuite::Testsuite(const char * suitename) : name(suitename) {
	next = tests;
	tests = this;
}

int Testsuite::run_all() {
	
	int count = 0;
	for(Testsuite * test = tests; test; test = test->next) {
		count++;
	}
	int len = 2;
	int r = count;
	while(r >= 10) {
		len++;
		r /= 10;
	}
	
	int i = 0;
	for(Testsuite * test = tests; test; test = test->next) {
		i++;
		if(test_verbose || test_progress) {
			std::printf("%*d/%d [%s]", len, i, count, test->name);
			if(test_verbose) {
				std::printf("\n");
			}
		}
		try {
			test->run();
		} catch(...) {
			test_failed++;
			if(test_progress) {
				std::printf("\r\x1b[K");
			}
			std::fprintf(stderr, "%s: EXCEPTION\n", test->name);
		}
	}
	
	if(test_progress) {
		std::printf("\r\x1b[K");
	}
	if(test_failed == 0) {
		std::printf("all %d test suites passed\n", count);
	}
	
	return test_failed > 0 ? 1 : 0;
}

void Testsuite::test(const char * testcase, bool ok) {
	if(!ok) {
		test_failed++;
	}
	if(test_progress) {
		std::printf("\r\x1b[K");
	}
	if(!ok || test_verbose) {
		std::fprintf(ok ? stdout : stderr, "%s.%s: %s\n", name, testcase, ok ? "ok" : "FAILED");
	}
}

int main(int argc, const char * argv[]) {
	
	if((argc > 1 && std::strcmp(argv[1], "--verbose") == 0) || \
	   (argc > 1 && argv[1][0] == '-' && argv[1][1] != '-' && std::strchr(argv[1], 'v')) || \
	   (std::getenv("VERBOSE") && std::strcmp(std::getenv("VERBOSE"), "0") != 0)) {
		test_verbose = true;
	} else {
		#if defined(_WIN32) || INNOEXTRACT_HAVE_ISATTY
		test_progress = isatty(1) && isatty(2);
		#endif
		if(test_progress) {
			char * term = std::getenv("TERM");
			if(!term || !std::strcmp(term, "dumb")) {
				test_progress = false; // Terminal does not support escape sequences
			}
		}
	}
	
	return Testsuite::run_all();
}

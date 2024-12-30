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

/*!
 * \file
 *
 * Test utility functions.
 */
#ifndef INNOEXTRACT_UTIL_TEST_HPP
#define INNOEXTRACT_UTIL_TEST_HPP

#ifdef INNOEXTRACT_BUILD_TESTS

#include <stddef.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

static const char * testdata = "The dhole (pronounced \"dole\") is also known as the Asiatic wild dog,"
                               " red dog, and whistling dog. It is about the size of a German shepherd but"
                               " looks more like a long-legged fox. This highly elusive and skilled jumper"
                               " is classified with wolves, coyotes, jackals, and foxes in the taxonomic"
                               " family Canidae.";
static const size_t testlen = std::strlen(testdata);

struct Testsuite {
	
	Testsuite(const char * suitename);
	
	static int run_all();
	
	void test(const char * testcase, bool ok);
	
	inline void test_equals(const char * testcase, const void * a, const void * b, size_t count) {
		test(testcase, std::memcmp(a, b, count) == 0);
	}
	
	virtual void run() = 0;
	
private:
	
	static Testsuite * tests;
	Testsuite * next;
	
protected:
	
	const char * name;
	
};

#define INNOEXTRACT_TEST(Name, ...) \
	struct Name ## _test : public Testsuite { \
		Name ## _test() : Testsuite(# Name) { } \
		void run(); \
	} test_ ## Name; \
	void Name ## _test::run() { __VA_ARGS__ }

#else

#define INNOEXTRACT_TEST(Name, ...)

#endif // INNOEXTRACT_BUILD_TESTS

#endif // INNOEXTRACT_UTIL_TEST_HPP

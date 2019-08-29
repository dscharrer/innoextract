/*
 * Copyright (C) 2011-2019 Daniel Scharrer
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

// Taken from Crypto++ and modified to fit the project.
// sha.cpp - modified by Wei Dai from Steve Reid's public domain sha1.c

#include "crypto/sha1.hpp"

#include "util/math.hpp"

namespace crypto {

void sha1_transform::init(hash_word * state) {
	state[0] = 0x67452301l;
	state[1] = 0xefcdab89l;
	state[2] = 0x98badcfel;
	state[3] = 0x10325476l;
	state[4] = 0xc3d2e1f0l;
}

void sha1_transform::transform(hash_word * state, const hash_word * data) {
	
	#define blk0(i) (W[i] = data[i])
	#define blk1(i) (W[i & 15] = util::rotl_fixed(W[(i + 13) & 15] ^ W[(i + 8) & 15] \
	                                              ^ W[(i + 2) & 15] ^ W[i & 15], 1))
	
	#define f1(x, y, z) (z ^ (x & (y ^ z)))
	#define f2(x, y, z) (x ^ y ^ z)
	#define f3(x, y, z) ((x & y) | (z & (x | y)))
	#define f4(x, y, z) (x ^ y ^ z)
	
	/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
	#define R0(v, w, x, y, z, i) \
		z += f1(w, x, y) + blk0(i) + 0x5A827999 + util::rotl_fixed(v, 5); \
		w = util::rotl_fixed(w, 30);
	#define R1(v, w, x, y, z, i) \
		z += f1(w, x, y) + blk1(i) + 0x5A827999 + util::rotl_fixed(v, 5); \
		w = util::rotl_fixed(w, 30);
	#define R2(v, w, x, y, z, i) \
		z += f2(w, x, y) + blk1(i) + 0x6ED9EBA1 + util::rotl_fixed(v, 5); \
		w = util::rotl_fixed(w, 30);
	#define R3(v, w, x, y, z, i) \
		z += f3(w, x, y) + blk1(i) + 0x8F1BBCDC + util::rotl_fixed(v, 5); \
		w = util::rotl_fixed(w, 30);
	#define R4(v, w, x, y, z, i) \
		z += f4(w, x, y) + blk1(i) + 0xCA62C1D6 + util::rotl_fixed(v, 5); \
		w = util::rotl_fixed(w, 30);
	
	hash_word W[16];
	
	/* Copy context->state[] to working vars */
	hash_word a = state[0];
	hash_word b = state[1];
	hash_word c = state[2];
	hash_word d = state[3];
	hash_word e = state[4];
	
	/* 4 rounds of 20 operations each. Loop unrolled. */
	
	R0(a, b, c, d, e,  0);
	R0(e, a, b, c, d,  1);
	R0(d, e, a, b, c,  2);
	R0(c, d, e, a, b,  3);
	
	R0(b, c, d, e, a,  4);
	R0(a, b, c, d, e,  5);
	R0(e, a, b, c, d,  6);
	R0(d, e, a, b, c,  7);
	
	R0(c, d, e, a, b,  8);
	R0(b, c, d, e, a,  9);
	R0(a, b, c, d, e, 10);
	R0(e, a, b, c, d, 11);
	
	R0(d, e, a, b, c, 12);
	R0(c, d, e, a, b, 13);
	R0(b, c, d, e, a, 14);
	R0(a, b, c, d, e, 15);
	
	R1(e, a, b, c, d, 16);
	R1(d, e, a, b, c, 17);
	R1(c, d, e, a, b, 18);
	R1(b, c, d, e, a, 19);
	
	R2(a, b, c, d, e, 20);
	R2(e, a, b, c, d, 21);
	R2(d, e, a, b, c, 22);
	R2(c, d, e, a, b, 23);
	
	R2(b, c, d, e, a, 24);
	R2(a, b, c, d, e, 25);
	R2(e, a, b, c, d, 26);
	R2(d, e, a, b, c, 27);
	
	R2(c, d, e, a, b, 28);
	R2(b, c, d, e, a, 29);
	R2(a, b, c, d, e, 30);
	R2(e, a, b, c, d, 31);
	
	R2(d, e, a, b, c, 32);
	R2(c, d, e, a, b, 33);
	R2(b, c, d, e, a, 34);
	R2(a, b, c, d, e, 35);
	
	R2(e, a, b, c, d, 36);
	R2(d, e, a, b, c, 37);
	R2(c, d, e, a, b, 38);
	R2(b, c, d, e, a, 39);
	
	R3(a, b, c, d, e, 40);
	R3(e, a, b, c, d, 41);
	R3(d, e, a, b, c, 42);
	R3(c, d, e, a, b, 43);
	
	R3(b, c, d, e, a, 44);
	R3(a, b, c, d, e, 45);
	R3(e, a, b, c, d, 46);
	R3(d, e, a, b, c, 47);
	
	R3(c, d, e, a, b, 48);
	R3(b, c, d, e, a, 49);
	R3(a, b, c, d, e, 50);
	R3(e, a, b, c, d, 51);
	
	R3(d, e, a, b, c, 52);
	R3(c, d, e, a, b, 53);
	R3(b, c, d, e, a, 54);
	R3(a, b, c, d, e, 55);
	
	R3(e, a, b, c, d, 56);
	R3(d, e, a, b, c, 57);
	R3(c, d, e, a, b, 58);
	R3(b, c, d, e, a, 59);
	
	R4(a, b, c, d, e, 60);
	R4(e, a, b, c, d, 61);
	R4(d, e, a, b, c, 62);
	R4(c, d, e, a, b, 63);
	
	R4(b, c, d, e, a, 64);
	R4(a, b, c, d, e, 65);
	R4(e, a, b, c, d, 66);
	R4(d, e, a, b, c, 67);
	
	R4(c, d, e, a, b, 68);
	R4(b, c, d, e, a, 69);
	R4(a, b, c, d, e, 70);
	R4(e, a, b, c, d, 71);
	
	R4(d, e, a, b, c, 72);
	R4(c, d, e, a, b, 73);
	R4(b, c, d, e, a, 74);
	R4(a, b, c, d, e, 75);
	
	R4(e, a, b, c, d, 76);
	R4(d, e, a, b, c, 77);
	R4(c, d, e, a, b, 78);
	R4(b, c, d, e, a, 79);
	
	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	
	#undef R4
	#undef R3
	#undef R2
	#undef R1
	#undef R0
	
	#undef f4
	#undef f3
	#undef f2
	#undef f1
	
	#undef blk1
	#undef blk0
	
}

} // namespace crypto

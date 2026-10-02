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

// Taken from Crypto++ and modified to fit the project.
// Wei Dai implemented SHA-2.

#include "crypto/sha256.hpp"

#include "util/math.hpp"
#include "util/test.hpp"

namespace crypto {

static const boost::uint32_t sha256_k[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void sha256_transform::init(hash_word * state) {
	state[0] = 0x6a09e667;
	state[1] = 0xbb67ae85;
	state[2] = 0x3c6ef372;
	state[3] = 0xa54ff53a;
	state[4] = 0x510e527f;
	state[5] = 0x9b05688c;
	state[6] = 0x1f83d9ab;
	state[7] = 0x5be0cd19;
}

void sha256_transform::transform(hash_word * state, const hash_word * data) {
	
	#define a(i) T[(0 - i) & 7]
	#define b(i) T[(1 - i) & 7]
	#define c(i) T[(2 - i) & 7]
	#define d(i) T[(3 - i) & 7]
	#define e(i) T[(4 - i) & 7]
	#define f(i) T[(5 - i) & 7]
	#define g(i) T[(6 - i) & 7]
	#define h(i) T[(7 - i) & 7]
	
	#define blk0(i) (W[i] = data[i])
	#define blk2(i) (W[i & 15] += s1(W[(i - 2) & 15]) + W[(i - 7) & 15] + s0(W[(i - 15) & 15]))
	
	#define Ch(x, y, z) (z ^ (x & (y ^ z)))
	#define Maj(x, y, z) (y ^ ((x ^ y) & (y ^ z)))
	
	#define R(i) \
		h(i) += S1(e(i)) + Ch(e(i), f(i), g(i)) + sha256_k[i + j] + (j ? blk2(i) : blk0(i)); \
		d(i) += h(i); \
		h(i) += S0(a(i)) + Maj(a(i), b(i), c(i))
	
	#define s0(x) (util::rotr_fixed(x, 7) ^ util::rotr_fixed(x, 18) ^ (x >> 3))
	#define s1(x) (util::rotr_fixed(x, 17) ^ util::rotr_fixed(x, 19) ^ (x >> 10))
	#define S0(x) (util::rotr_fixed(x, 2) ^ util::rotr_fixed(x, 13) ^ util::rotr_fixed(x, 22))
	#define S1(x) (util::rotr_fixed(x, 6) ^ util::rotr_fixed(x, 11) ^ util::rotr_fixed(x, 25))
	
	hash_word W[16] = {0}, T[8];
	
	/* Copy context->state to working vars */
	std::memcpy(T, state, sizeof(T));
	
	/* 64 operations, partially loop unrolled */
	for(size_t j = 0; j < 64; j += 16) {
		R(0);
		R(1);
		R(2);
		R(3);
		R(4);
		R(5);
		R(6);
		R(7);
		R(8);
		R(9);
		R(10);
		R(11);
		R(12);
		R(13);
		R(14);
		R(15);
	}
	
	/* Add the working vars back into context.state[] */
	state[0] += a(0);
	state[1] += b(0);
	state[2] += c(0);
	state[3] += d(0);
	state[4] += e(0);
	state[5] += f(0);
	state[6] += g(0);
	state[7] += h(0);
	
	#undef Ch
	#undef Maj
	#undef s0
	#undef s1
	#undef S0
	#undef S1
	#undef blk0
	#undef blk1
	#undef blk2
	#undef R
	
	#undef a
	#undef b
	#undef c
	#undef d
	#undef e
	#undef f
	#undef g
	#undef h
	
}

INNOEXTRACT_TEST(sha256,
	
	const boost::uint8_t expected[] = {
		0x20, 0x95, 0xa0, 0x2a, 0x36, 0x55, 0x64, 0x28, 0xa4, 0x50, 0xe7, 0x7a, 0xbf, 0x44, 0x96, 0x1e,
		0x8a, 0xf1, 0xee, 0xb2, 0x0b, 0x92, 0xc3, 0x8b, 0xbd, 0x7b, 0x83, 0xd0, 0x08, 0xd7, 0xe1, 0xfa
	};
	
	sha256 checksum;
	checksum.init();
	checksum.update(testdata, testlen);
	char buffer[32];
	checksum.finalize(buffer);
	test_equals("checksum", buffer, expected, sizeof(expected));
	
)

} // namespace crypto

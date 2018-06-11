/*
 * Copyright (C) 2018 Daniel Scharrer
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

#include "crypto/arc4.hpp"

#include <algorithm>

#include "util/endian.hpp"

namespace crypto {

void arc4::init(const char * key, size_t length) {
	
	a = b = 0;
	
	for(size_t i = 0; i < sizeof(state); i++){
		state[i] = boost::uint8_t(i);
	}
	
	size_t j = 0;
	for(size_t i = 0; i < sizeof(state); i++) {
		j = (j + state[i] + boost::uint8_t(key[i % length])) % sizeof(state);
		std::swap(state[i], state[j]);
	}
	
}

void arc4::update() {
	
	a = (a + 1) % sizeof(state);
	b = (b + state[a]) % sizeof(state);
	
	std::swap(state[a], state[b]);
	
}

void arc4::discard(size_t length) {
	
	for(size_t i = 0; i < length; i++) {
		update();
	}
	
}

void arc4::crypt(const char * in, char * out, size_t length) {
	
	for(size_t i = 0; i < length; i++) {
		update();
		out[i] = char(state[size_t(state[a] + state[b]) % sizeof(state)] ^ boost::uint8_t(in[i]));
	}
	
}

} // namespace crypto

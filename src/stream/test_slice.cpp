/*
 * Copyright (C) 2026 Daniel Scharrer
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

#include "stream/slice.hpp"

#include "util/test.hpp"

namespace stream {

INNOEXTRACT_TEST(slice,
	
	const char idska16[8] = { 'i', 'd', 's', 'k', 'a', '1', '6', 0x1a };
	const char idska32[8] = { 'i', 'd', 's', 'k', 'a', '3', '2', 0x1a };
	const char idskb32[8] = { 'i', 'd', 's', 'k', 'b', '3', '2', 0x1a };
	
	// Inno Setup <= 6.5.x stores a 32-bit size after the slice id.
	test("idska16 uses a 32-bit size", slice_size_field_width(idska16) == 4);
	test("idska32 uses a 32-bit size", slice_size_field_width(idska32) == 4);
	
	// Inno Setup >= 6.6.0 stores a 64-bit size after the slice id.
	test("idskb32 uses a 64-bit size", slice_size_field_width(idskb32) == 8);
	
	// Anything else must be rejected instead of being searched for out of bounds.
	const char bad_letter[8] = { 'i', 'd', 's', 'k', 'c', '3', '2', 0x1a };
	const char bad_magic[8] = { 'i', 'd', 's', 'k', 'b', '3', '2', 0x00 };
	const char bad_prefix[8] = { 'i', 'd', 's', 'x', 'b', '3', '2', 0x1a };
	const char bad_empty[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	
	test("unknown slice variant is rejected", slice_size_field_width(bad_letter) == 0);
	test("slice id needs the 0x1a terminator", slice_size_field_width(bad_magic) == 0);
	test("slice id needs the idsk prefix", slice_size_field_width(bad_prefix) == 0);
	test("all-zero magic is rejected", slice_size_field_width(bad_empty) == 0);
	
)

} // namespace stream

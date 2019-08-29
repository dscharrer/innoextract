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
// crc.cpp - written and placed in the public domain by Wei Dai

#include "crypto/crc32.hpp"

#include "util/endian.hpp"

namespace crypto {

/* Table of CRC-32's of all single byte values (made by makecrc.c) */
static const boost::uint32_t crc32_table[] = {
	0x00000000l, 0x77073096l, 0xee0e612cl, 0x990951bal, 0x076dc419l,
	0x706af48fl, 0xe963a535l, 0x9e6495a3l, 0x0edb8832l, 0x79dcb8a4l,
	0xe0d5e91el, 0x97d2d988l, 0x09b64c2bl, 0x7eb17cbdl, 0xe7b82d07l,
	0x90bf1d91l, 0x1db71064l, 0x6ab020f2l, 0xf3b97148l, 0x84be41del,
	0x1adad47dl, 0x6ddde4ebl, 0xf4d4b551l, 0x83d385c7l, 0x136c9856l,
	0x646ba8c0l, 0xfd62f97al, 0x8a65c9ecl, 0x14015c4fl, 0x63066cd9l,
	0xfa0f3d63l, 0x8d080df5l, 0x3b6e20c8l, 0x4c69105el, 0xd56041e4l,
	0xa2677172l, 0x3c03e4d1l, 0x4b04d447l, 0xd20d85fdl, 0xa50ab56bl,
	0x35b5a8fal, 0x42b2986cl, 0xdbbbc9d6l, 0xacbcf940l, 0x32d86ce3l,
	0x45df5c75l, 0xdcd60dcfl, 0xabd13d59l, 0x26d930acl, 0x51de003al,
	0xc8d75180l, 0xbfd06116l, 0x21b4f4b5l, 0x56b3c423l, 0xcfba9599l,
	0xb8bda50fl, 0x2802b89el, 0x5f058808l, 0xc60cd9b2l, 0xb10be924l,
	0x2f6f7c87l, 0x58684c11l, 0xc1611dabl, 0xb6662d3dl, 0x76dc4190l,
	0x01db7106l, 0x98d220bcl, 0xefd5102al, 0x71b18589l, 0x06b6b51fl,
	0x9fbfe4a5l, 0xe8b8d433l, 0x7807c9a2l, 0x0f00f934l, 0x9609a88el,
	0xe10e9818l, 0x7f6a0dbbl, 0x086d3d2dl, 0x91646c97l, 0xe6635c01l,
	0x6b6b51f4l, 0x1c6c6162l, 0x856530d8l, 0xf262004el, 0x6c0695edl,
	0x1b01a57bl, 0x8208f4c1l, 0xf50fc457l, 0x65b0d9c6l, 0x12b7e950l,
	0x8bbeb8eal, 0xfcb9887cl, 0x62dd1ddfl, 0x15da2d49l, 0x8cd37cf3l,
	0xfbd44c65l, 0x4db26158l, 0x3ab551cel, 0xa3bc0074l, 0xd4bb30e2l,
	0x4adfa541l, 0x3dd895d7l, 0xa4d1c46dl, 0xd3d6f4fbl, 0x4369e96al,
	0x346ed9fcl, 0xad678846l, 0xda60b8d0l, 0x44042d73l, 0x33031de5l,
	0xaa0a4c5fl, 0xdd0d7cc9l, 0x5005713cl, 0x270241aal, 0xbe0b1010l,
	0xc90c2086l, 0x5768b525l, 0x206f85b3l, 0xb966d409l, 0xce61e49fl,
	0x5edef90el, 0x29d9c998l, 0xb0d09822l, 0xc7d7a8b4l, 0x59b33d17l,
	0x2eb40d81l, 0xb7bd5c3bl, 0xc0ba6cadl, 0xedb88320l, 0x9abfb3b6l,
	0x03b6e20cl, 0x74b1d29al, 0xead54739l, 0x9dd277afl, 0x04db2615l,
	0x73dc1683l, 0xe3630b12l, 0x94643b84l, 0x0d6d6a3el, 0x7a6a5aa8l,
	0xe40ecf0bl, 0x9309ff9dl, 0x0a00ae27l, 0x7d079eb1l, 0xf00f9344l,
	0x8708a3d2l, 0x1e01f268l, 0x6906c2fel, 0xf762575dl, 0x806567cbl,
	0x196c3671l, 0x6e6b06e7l, 0xfed41b76l, 0x89d32be0l, 0x10da7a5al,
	0x67dd4accl, 0xf9b9df6fl, 0x8ebeeff9l, 0x17b7be43l, 0x60b08ed5l,
	0xd6d6a3e8l, 0xa1d1937el, 0x38d8c2c4l, 0x4fdff252l, 0xd1bb67f1l,
	0xa6bc5767l, 0x3fb506ddl, 0x48b2364bl, 0xd80d2bdal, 0xaf0a1b4cl,
	0x36034af6l, 0x41047a60l, 0xdf60efc3l, 0xa867df55l, 0x316e8eefl,
	0x4669be79l, 0xcb61b38cl, 0xbc66831al, 0x256fd2a0l, 0x5268e236l,
	0xcc0c7795l, 0xbb0b4703l, 0x220216b9l, 0x5505262fl, 0xc5ba3bbel,
	0xb2bd0b28l, 0x2bb45a92l, 0x5cb36a04l, 0xc2d7ffa7l, 0xb5d0cf31l,
	0x2cd99e8bl, 0x5bdeae1dl, 0x9b64c2b0l, 0xec63f226l, 0x756aa39cl,
	0x026d930al, 0x9c0906a9l, 0xeb0e363fl, 0x72076785l, 0x05005713l,
	0x95bf4a82l, 0xe2b87a14l, 0x7bb12bael, 0x0cb61b38l, 0x92d28e9bl,
	0xe5d5be0dl, 0x7cdcefb7l, 0x0bdbdf21l, 0x86d3d2d4l, 0xf1d4e242l,
	0x68ddb3f8l, 0x1fda836el, 0x81be16cdl, 0xf6b9265bl, 0x6fb077e1l,
	0x18b74777l, 0x88085ae6l, 0xff0f6a70l, 0x66063bcal, 0x11010b5cl,
	0x8f659effl, 0xf862ae69l, 0x616bffd3l, 0x166ccf45l, 0xa00ae278l,
	0xd70dd2eel, 0x4e048354l, 0x3903b3c2l, 0xa7672661l, 0xd06016f7l,
	0x4969474dl, 0x3e6e77dbl, 0xaed16a4al, 0xd9d65adcl, 0x40df0b66l,
	0x37d83bf0l, 0xa9bcae53l, 0xdebb9ec5l, 0x47b2cf7fl, 0x30b5ffe9l,
	0xbdbdf21cl, 0xcabac28al, 0x53b39330l, 0x24b4a3a6l, 0xbad03605l,
	0xcdd70693l, 0x54de5729l, 0x23d967bfl, 0xb3667a2el, 0xc4614ab8l,
	0x5d681b02l, 0x2a6f2b94l, 0xb40bbe37l, 0xc30c8ea1l, 0x5a05df1bl,
	0x2d02ef8dl
};

static boost::uint8_t crc32_index(boost::uint32_t crc) {
	return boost::uint8_t(crc & 0xff);
}

static boost::uint32_t crc32_shifted(boost::uint32_t crc) {
	return crc >> 8;
}

void crc32::update(const char * data, size_t length) {
	
	for(; (size_t(data) % 4 != 0) && length > 0; length--) {
		crc = crc32_table[crc32_index(crc) ^ boost::uint8_t(*data++)] ^ crc32_shifted(crc);
	}
	
	while(length >= 4) {
		crc ^= util::little_endian::load<boost::uint32_t>(data);
		crc = crc32_table[crc32_index(crc)] ^ crc32_shifted(crc);
		crc = crc32_table[crc32_index(crc)] ^ crc32_shifted(crc);
		crc = crc32_table[crc32_index(crc)] ^ crc32_shifted(crc);
		crc = crc32_table[crc32_index(crc)] ^ crc32_shifted(crc);
		length -= 4;
		data += 4;
	}
	
	while(length--) {
		crc = crc32_table[crc32_index(crc) ^ boost::uint8_t(*data++)] ^ crc32_shifted(crc);
	}
	
}

} // namespace crypto

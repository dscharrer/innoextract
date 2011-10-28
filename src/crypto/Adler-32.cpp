
// Taken from Crypto++ and modified to fit the project.
// adler32.cpp - written and placed in the public domain by Wei Dai

#include "crypto/Adler-32.hpp"

void Adler32::update(const char * input, size_t length) {
	
	const unsigned long BASE = 65521;
	
	unsigned long s1 = this->s1;
	unsigned long s2 = this->s2;
	
	if(length % 8 != 0) {
		
		do {
			s1 += uint8_t(*input++);
			s2 += s1;
			length--;
		} while(length % 8 != 0);
		
		if(s1 >= BASE) {
			s1 -= BASE;
		}
		
		s2 %= BASE;
	}
	
	while(length > 0) {
		
		s1 += uint8_t(input[0]); s2 += s1;
		s1 += uint8_t(input[1]); s2 += s1;
		s1 += uint8_t(input[2]); s2 += s1;
		s1 += uint8_t(input[3]); s2 += s1;
		s1 += uint8_t(input[4]); s2 += s1;
		s1 += uint8_t(input[5]); s2 += s1;
		s1 += uint8_t(input[6]); s2 += s1;
		s1 += uint8_t(input[7]); s2 += s1;
		
		length -= 8;
		input += 8;
		
		if(s1 >= BASE) {
			s1 -= BASE;
		}
		
		if(length % 0x8000 == 0) {
			s2 %= BASE;
		}
	}
	
	this->s1 = uint16_t(s1);
	this->s2 = uint16_t(s2);
}

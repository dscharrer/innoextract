
#include "src/SetupHeaderFormat.hpp"

#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::setfill;
using std::setw;
using std::hex;
using std::dec;

/*

template <size_t N>
void testlog() {
	cout << "  lnpot: " << setfill(' ') << setw(3)  << log_next_power_of_two<N>::value;
}

template <>
void testlog<0>() {
	cout << "  lnpot: " << setfill(' ') << setw(3)  << 0;
}

template <size_t N>
void test() {
	
	cout << setfill(' ') << setw(5) << N;
	cout << ' ' << hex << setfill('0') << setw(4) << N << dec;
	cout << ":  pot=" << is_power_of_two<N>::value;
	cout << "  npot: " << setfill(' ') << setw(5) << next_power_of_two<N>::value;
	testlog<N>();
	cout << "  ebits: " << setfill(' ') << setw(3) << StoredEnumType<N>::bits;
	cout << "  rebits: " << setfill(' ') << setw(3) << (sizeof(typename StoredEnumType<N>::type) * 8);
	cout << "  fbits: " << setfill(' ') << setw(5) << StoredFlagType<N>::bits;
	cout << "  rfbits: " << setfill(' ') << setw(5) << (sizeof(typename StoredFlagType<N>::type) * 8);
	
	cout << endl;
}

#define TEST0(D) test<0##D>();

#define TEST1(D) \
	TEST0(D##0) \
	TEST0(D##1) \
	TEST0(D##2) \
	TEST0(D##3) \
	TEST0(D##4) \
	TEST0(D##5) \
	TEST0(D##6) \
	TEST0(D##7) \

#define TEST2(D) \
	TEST1(D##0) \
	TEST1(D##1) \
	TEST1(D##2) \
	TEST1(D##3) \
	TEST1(D##4) \
	TEST1(D##5) \
	TEST1(D##6) \
	TEST1(D##7) \

#define TEST3(D) \
	TEST2(D##0) \
	TEST2(D##1) \
	TEST2(D##2) \
	TEST2(D##3) \
	TEST2(D##4) \
	TEST2(D##5) \
	TEST2(D##6) \
	TEST2(D##7) \

enum TestEnum {
	
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	
	TestEnum__End
};
ENUM_SIZE_AUTO(TestEnum);

STORED_ENUM_MAP(TestEnumMap, A,
	A, // 0
	B, // 1
	C, // 2
	D, // 2
	E, // 2
	F, // 2
	G, // 2
	H, // 2
	I, // 2
	J, // 2
	K, // 2
	L, // 2
	M, // 2
	N, // 2
	O, // 2
	P, // 2
	Q, // 2
	R, // 2
	S, // 2
	T, // 2
	U, // 2
	V, // 2
	W, // 2
	X, // 2
	Y, // 2
	Z, // 2
);
*/

volatile size_t i;

typedef BitsetConverter
	::add::map<0, 0>
	::add::map<1, 1>
	::add::map<2, 2>
	::add::map<3, 3>
	::add::map<4, 4>
	::add::map<5, 5>
	::add::map<6, 6>
	::add::map<7, 7>
	::add::map<8, 8>
	::add::map<9, 9>
	::add::map<10, 10>
	::add::map<11, 11>
	::add::map<12, 12>
	::add::map<13, 13>
	::add::map<14, 14>
	::add::map<15, 15>
	::add::map<16, 16>
	::add::map<17, 17>
	::add::map<18, 18>
	::add::map<19, 19>
	::add::map<20, 20>
	IdentityConverter;

static void test2(size_t i) {
	
	size_t out = IdentityConverter::convert(i);
	
	cout << i << " = " << std::bitset<64>(i) << " -> " << out << " = " << std::bitset<64>(out) << endl;
	
}

int main() {
	
	 //TEST3()
	
	/*
	test2(0);
	test2(1);
	test2(2);
	test2(3);
	test2(4);
	test2(5);
	test2(6);
	test2(7);
	test2(8);*/
	
	
	return IdentityConverter::convert(i);
}


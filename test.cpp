
#include "src/SetupHeaderFormat.hpp"

#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::setfill;
using std::setw;
using std::hex;
using std::dec;

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

void test2(size_t i) {
	
	StoredFlags<TestEnumMap> test(i);
	
	cout << i << " = " << std::bitset<4>(i) << " -> " << test.get() << endl;
	
}

volatile size_t i;

typedef MapList
	::add<0, 0>::list
	::add<1, 1>::list
	::add<2, 2>::list
	::add<3, 3>::list
	::add<4, 4>::list
	::add<5, 5>::list
	::add<6, 6>::list
	::add<7, 7>::list
	::add<8, 8>::list
	::add<9, 9>::list
	::add<10, 10>::list
	::add<11, 11>::list
	::add<12, 12>::list
	::add<13, 13>::list
	::add<14, 14>::list
	::add<15, 15>::list
	::add<16, 16>::list
	::add<17, 17>::list
	::add<18, 18>::list
	::add<19, 19>::list
	::add<20, 20>::list
	IdentityList;

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
	test2(8); */
	
	
	return Evaluator<IdentityList>::map(i);
}


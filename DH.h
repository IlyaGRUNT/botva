#pragma once
#include <vector>

using namespace std;

namespace DH {
	constexpr array<uint8_t, 16> primeSet{ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53 };
	unsigned short powMod(unsigned short a, unsigned short b, unsigned short p);
	unsigned short fi(double num);
	unsigned short gcd(unsigned short a, unsigned short b);
	vector<unsigned short> getRequiredSet(unsigned short modulo);
	vector<unsigned short> getActualSet(uint8_t g, unsigned short modulo);
	uint8_t primeRoot(uint8_t modulo);
	unsigned short getPublicKey1(unsigned short p, unsigned short privateKey);
	unsigned short getMasterKey(unsigned short p, unsigned short privateKey, unsigned short publicKey2);
}
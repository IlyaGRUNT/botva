#include <vector>
#pragma once

using namespace std;

namespace DH {
	unsigned short fi(double num);
	unsigned short gcd(unsigned short a, unsigned short b);
	vector<uint8_t> getRequiredSet(uint8_t modulo);
	vector<uint8_t> getActualSet(uint8_t g, uint8_t modulo);
	uint8_t primeRoot(uint8_t modulo);
	unsigned short getPublicKey1(uint8_t p, uint8_t privateKey);
	unsigned short getMasterKey(uint8_t p, uint8_t privateKey, unsigned short publicKey2);
}
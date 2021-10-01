#include <string>
#include <array>
#include <cmath>
#include <vector>
#include "DH.h"

using namespace std;

namespace DH {
	constexpr array<uint8_t, 25> primeSet{ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 };

	unsigned short fi(double num) {
		double result{ num };
		unsigned short i{ 2 };
		while (pow(i, 2) < num) {
			while (static_cast<unsigned short>(num) % i == 0) {
				num /= i;
				result -= result / i;
			}
			i += 1;
		}
		if (num > 1)
			result -= result / i;
		return static_cast<unsigned short>(result);
	}

	unsigned short gcd(unsigned short a, unsigned short b) {
		while (a != b) {
			if (a > b)
				a -= b;
			else
				b -= a;
		}
		return a;
	}

	vector<uint8_t> getRequiredSet(uint8_t modulo) {
		vector<uint8_t> result{};
		for (uint8_t num = 1; num < modulo; num++)
			if (gcd(num, modulo) == 1)
				result.insert(result.end(), num);
		return result;
	}

	vector<uint8_t> getActualSet(uint8_t g, uint8_t modulo) {
		vector<uint8_t> result{};
		for (uint8_t power = 1; power < modulo; power++)
			result.insert(result.end(), static_cast<unsigned int>(pow(g, power)) % modulo);
		return result;
	}

	uint8_t primeRoot(uint8_t modulo) {
		vector<uint8_t> requiredSet{ getRequiredSet(modulo) };
		vector<uint8_t> actualSet{};
		for (uint8_t g = 0; g < modulo; g++) {
			actualSet = getActualSet(g, modulo);
			if (actualSet == requiredSet)
				return g;
		}
	}

	unsigned short getPublicKey1(uint8_t p, uint8_t privateKey) {
		uint8_t g{ primeRoot(p) };
		unsigned short publicKey1{ static_cast<unsigned short>(pow(g, privateKey)) % p };
		return publicKey1;
	}

	unsigned short getMasterKey(uint8_t p, uint8_t privateKey, unsigned short publicKey2) {
		unsigned short masterKey{ static_cast<unsigned short>(pow(publicKey2, privateKey)) % p };
		return masterKey;
	}
}
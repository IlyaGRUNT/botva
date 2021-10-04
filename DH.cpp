#include <string>
#include <array>
#include <cmath>
#include <vector>
#include "DH.h"

using namespace std;

namespace DH {
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

	vector<unsigned short> getRequiredSet(unsigned short modulo) {
		vector<unsigned short> result{};
		for (unsigned short num = 1; num < modulo; num++)
			if (gcd(num, modulo) == 1)
				result.insert(result.end(), num);
		return result;
	}

	vector<unsigned short> getActualSet(uint8_t g, unsigned short modulo) {
		vector<unsigned short> result{};
		for (uint8_t power = 1; power < modulo; power++)
			result.insert(result.end(), powMod(g, power, modulo));
		return result;
	}

	unsigned short powMod(unsigned short a, unsigned short b, unsigned short p) {
		int res = 1;
		while (b)
			if (b & 1)
				res = int(res * 1ll * a % p), --b;
			else
				a = int(a * 1ll * a % p), b >>= 1;
		return res;
	}

	uint8_t primeRoot(uint8_t modulo) {
		vector<int> fact;
		int phi = modulo - 1, n = phi;
		for (int i = 2; i * i <= n; ++i)
			if (n % i == 0) {
				fact.push_back(i);
				while (n % i == 0)
					n /= i;
			}
		if (n > 1)
			fact.push_back(n);

		for (int res = 2; res <= modulo; ++res) {
			bool ok = true;
			for (size_t i = 0; i < fact.size() && ok; ++i)
				ok &= powMod(res, phi / fact[i], modulo) != 1;
			if (ok)  return res;
		}
	}

	unsigned short getPublicKey1(unsigned short p, unsigned short privateKey) {
		const uint8_t g{ primeRoot(p) };
		unsigned short publicKey1{ powMod(primeRoot(p), privateKey, p)};
		return publicKey1;
	}

	unsigned short getMasterKey(unsigned short p, unsigned short privateKey, unsigned short publicKey2) {
		const unsigned short masterKey{ powMod(publicKey2, privateKey, p) };
		return masterKey;
	}
}
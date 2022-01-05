#include <string>
#include <array>
#include <cmath>
#include <vector>
#include <sstream>
#include <random>
#include "DH.h"

using namespace std;

namespace DH {
	string dec_to_hex(unsigned short dec) {
		stringstream ss;
		ss << hex << dec;
		return ss.str();
	}

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

	vector<unsigned short> get_required_set(unsigned short modulo) {
		vector<unsigned short> result{};
		for (unsigned short num = 1; num < modulo; num++)
			if (gcd(num, modulo) == 1)
				result.insert(result.end(), num);
		return result;
	}

	vector<unsigned short> get_actual_set(uint8_t g, unsigned short modulo) {
		vector<unsigned short> result{};
		for (uint8_t power = 1; power < modulo; power++)
			result.insert(result.end(), pow_mod(g, power, modulo));
		return result;
	}

	unsigned short pow_mod(unsigned short a, unsigned long long b, unsigned short p) {
		int res = 1;
		while (b)
			if (b & 1)
				res = int(res * 1ll * a % p), --b;
			else
				a = int(a * 1ll * a % p), b >>= 1;
		return res;
	}

	uint8_t prime_root(unsigned short modulo) {
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
				ok &= pow_mod(res, phi / fact[i], modulo) != 1;
			if (ok)  return res;
		}
		return 0;
	}

	unsigned long long get_private_key() {
		default_random_engine generator;
		uniform_int_distribution<unsigned long long> distribution(1, maxULLong);
		return distribution(generator);
	}

	array<unsigned short, 64> get_p_arr() {
		array<unsigned short, 64> p_arr{};
		for (uint8_t i = 0; i < 64; i++)
			p_arr[i] = prime_set[rand() % 168];
		return p_arr;
	}

	array<unsigned long long, 64> get_private_key_arr() {
		array<unsigned long long, 64> privateKeyArr{};
		for (uint8_t i = 0; i < 64; i++)
			privateKeyArr[i] = get_private_key();
		return privateKeyArr;
	}

	array<unsigned short, 64> get_public_key_arr(array<unsigned short, 64> p_arr, array<unsigned long long, 64> private_key_arr) {
		array<unsigned short, 64> public_key_arr{};
		for (uint8_t i = 0; i < 64; i++)
			public_key_arr[i] = get_public_key(p_arr[i], private_key_arr[i]);
		return public_key_arr;
	}

	unsigned short get_public_key(unsigned short p, unsigned long long private_key) {
		const uint8_t g{ prime_root(p) };
		unsigned short public_key{ pow_mod(g, private_key, p)};
		return public_key;
	}

	unsigned short get_master_key(unsigned short p, unsigned long long private_key, unsigned short public_key) {
		const unsigned short masterKey{ pow_mod(public_key, private_key, p) };
		return masterKey;
	}

	string get_AES_key(array<unsigned short, 64> p_arr, array<unsigned long long, 64> private_key_arr, array<unsigned short, 64> public_key_arr) {
		string AES_key{};
		for (uint8_t i = 0; i < 64; i++)
			AES_key += dec_to_hex(get_master_key(p_arr[i], private_key_arr[i], public_key_arr[i]));
		AES_key.erase(64, string::npos);
		return AES_key;
	}
}
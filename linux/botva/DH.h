#pragma once
#include <vector>
#include <string>

namespace DH {
	constexpr std::array<unsigned short, 168> prime_set{ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997};
	constexpr unsigned long long maxULLong{ 18446744073709551615ULL };
	std::string dec_to_hex(unsigned short dec);
	unsigned short pow_mod(unsigned short a, unsigned long long b, unsigned short p);
	unsigned short fi(double num);
	unsigned short gcd(unsigned short a, unsigned short b);
	std::vector<unsigned short> get_required_set(unsigned short modulo);
	std::vector<unsigned short> get_actual_set(uint8_t g, unsigned short modulo);
	std::uint8_t prime_root(unsigned short modulo);
	unsigned long long get_private_key();
	std::array<unsigned short, 64> get_p_arr(); 
	std::array<unsigned long long, 64> get_private_key_arr();
	std::array<unsigned short, 64> get_public_key_arr(std::array<unsigned short, 64> pArr, std::array<unsigned long long, 64> private_key_arr);
	unsigned short get_public_key(unsigned short p, unsigned long long privateKey);
	unsigned short get_master_key(unsigned short p, unsigned long long privateKey, unsigned short publicKey2);
	std::string get_AES_key(std::array<unsigned short, 64> p_arr, std::array<unsigned long long, 64> private_key_arr, std::array<unsigned short, 64> public_key_arr);
}
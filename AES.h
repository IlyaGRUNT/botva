#pragma once
#include <string>
#include <array>
using namespace std;

namespace AES {
	bitset<8> galMul(bitset<8> a1, bitset<8> b1);
	bitset<4> hexCharToBin(char c);
	bitset<8> concat(bitset<4> a, bitset<4> b);
	bitset<8> hexToBin(string hexByte);
	string stringToHex(string str);
	array<bitset<8>, 4> xorBytes(array<bitset<8>, 4> bytes1, array<bitset<8>, 4> bytes2);
	void cycleShiftLeft(uint8_t rowNum);
	void cycleShiftRight(uint8_t rowNum);
	void subBytes();
	uint8_t getNumOfBlocks(string text);
	string getStrState(string text, unsigned int index);
	void getState(string text, unsigned int index);
	void shiftRows();
	void invShiftRows();
	bitset<8> mixColumnsMultiply(array<bitset<8>, 4> byte, uint8_t index);
	bitset<8> invMixColumnsMultiply(array<bitset<8>, 4> byte, uint8_t index);
	void mixColumns();
	void invMixColumns();
	void initKey(string key);
	array<bitset<8>, 4> g(array<bitset<8>, 4> word, uint8_t round);
	void expandKey(string key);
	void addRoundKey(uint8_t round);
	string stateToStr();
	string encrypt(string key, string text);
	string decrypt(string key, string text);
}
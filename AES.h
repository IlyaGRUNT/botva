#pragma once
#pragma once
#include <string>
#include <array>
using namespace std;

namespace AES {
	bitset<4> hexCharToBin(char c);
	string stringToHex(string str);
	bitset<8> concat(bitset<4> a, bitset<4> b);
	bitset<8> hexToBin(string hexByte);
	array<bitset<8>, 4>& xorBytes(array<bitset<8>, 4> bytes1, array<bitset<8>, 4> bytes2);
	void cycleShiftLeft(unsigned short rowNum);
	void subBytes();
	unsigned short getNumOfBlocks(string text);
	string getStrState(string text, unsigned int index);
	void getState(string text, unsigned int index);
	void shiftRows();
	bitset<8> mixColumnsMultiply(array<bitset<8>, 4> byte, unsigned short index);
	void mixColumns();
	void initKey(string key);
	array<bitset<8>, 4>& g(array<bitset<8>, 4> word, unsigned short round);
	void expandKey(string key);
	void addRoundKey(unsigned short round);
	string stateToStr();
	string deleteSpaces(string block);
	string encrypt(string key, string text);
}
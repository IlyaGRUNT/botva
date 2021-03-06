#pragma once
#include <string>
#include <array>
#include <bitset>

namespace AES {
	std::bitset<8> galMul(std::bitset<8> a1, std::bitset<8> b1);
	std::bitset<4> hexCharToBin(char c);
	std::bitset<8> concat(std::bitset<4> a, std::bitset<4> b);
	std::bitset<8> hexToBin(std::string hexByte);
	std::string stringToHex(std::string str);
	std::array<std::bitset<8>, 4> xorBytes(std::array<std::bitset<8>, 4> bytes1, std::array<std::bitset<8>, 4> bytes2);
	void cycleShiftLeft(uint8_t rowNum);
	void cycleShiftRight(uint8_t rowNum);
	void subBytes();
	uint8_t getNumOfBlocks(std::string text);
	std::string getStrState(std::string text, unsigned int index);
	void getState(std::string text, unsigned int index);
	void shiftRows();
	void invShiftRows();
	std::bitset<8> mixColumnsMultiply(std::array<std::bitset<8>, 4> byte, uint8_t index);
	std::bitset<8> invMixColumnsMultiply(std::array<std::bitset<8>, 4> byte, uint8_t index);
	void mixColumns();
	void invMixColumns();
	void initKey(std::string key);
	std::array<std::bitset<8>, 4> g(std::array<std::bitset<8>, 4> word, uint8_t round);
	void expandKey(std::string key);
	void addRoundKey(uint8_t round);
	std::string stateToStr();
	std::string encrypt(std::string key, std::string text);
	std::string decrypt(std::string key, std::string text);
}
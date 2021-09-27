#include <iostream>
#include <cstdint>
#include <bitset>
#include <array>
#include <sstream>
#include <string>
#include "AES.h"

using namespace std;

namespace AES {
    typedef uint8_t gal8;
    const gal8 min_poly = 0b11101,
        generator = 0b10;
    const array<array<string, 16>, 16> SBox{
        array<string, 16> {"63", "7c", "77", "7b", "f2", "6b", "6f", "c5", "30", "01", "67", "2b", "fe", "d7", "ab", "76"},
        array<string, 16> {"ca", "82", "c9", "7d", "fa", "59", "47", "f0", "ad", "d4", "a2", "af", "9c", "a4", "72", "c0"},
        array<string, 16> {"b7", "fd", "93", "26", "36", "3f", "f7", "cc", "34", "a5", "e5", "f1", "71", "d8", "31", "15"},
        array<string, 16> {"04", "c7", "23", "c3", "18", "96", "05", "9a", "07", "12", "80", "e2", "eb", "27", "b2", "75"},
        array<string, 16> {"09", "83", "2c", "1a", "1b", "6e", "5a", "a0", "52", "3b", "d6", "b3", "29", "e3", "2f", "84"},
        array<string, 16> {"53", "d1", "00", "ed", "20", "fc", "b1", "5b", "6a", "cb", "be", "39", "4a", "4c", "58", "cf"},
        array<string, 16> {"d0", "ef", "aa", "fb", "43", "4d", "33", "85", "45", "f9", "02", "7f", "50", "3c", "9f", "a8"},
        array<string, 16> {"51", "a3", "40", "8f", "92", "9d", "38", "f5", "bc", "b6", "da", "21", "10", "ff", "f3", "d2"},
        array<string, 16> {"cd", "0c", "13", "ec", "5f", "97", "44", "17", "c4", "a7", "7e", "3d", "64", "5d", "19", "73"},
        array<string, 16> {"60", "81", "4f", "dc", "22", "2a", "90", "88", "46", "ee", "b8", "14", "de", "5e", "0b", "db"},
        array<string, 16> {"e0", "32", "3a", "0a", "49", "06", "24", "5c", "c2", "d3", "ac", "62", "91", "95", "e4", "79"},
        array<string, 16> {"e7", "c8", "37", "6d", "8d", "d5", "4e", "a9", "6c", "56", "f4", "ea", "65", "7a", "ae", "08"},
        array<string, 16> {"ba", "78", "25", "2e", "1c", "a6", "b4", "c6", "e8", "dd", "74", "1f", "4b", "bd", "8b", "8a"},
        array<string, 16> {"70", "3e", "b5", "66", "48", "03", "f6", "0e", "61", "35", "57", "b9", "86", "c1", "1d", "9e"},
        array<string, 16> {"e1", "f8", "98", "11", "69", "d9", "8e", "94", "9b", "1e", "87", "e9", "ce", "55", "28", "df"},
        array<string, 16> {"8c", "a1", "89", "0d", "bf", "e6", "42", "68", "41", "99", "2d", "0f", "b0", "54", "bb", "16"}
    };
    const array<array<string, 16>, 16> invSBox{
        array<string, 16> {"52", "09", "6a", "d5", "30", "36", "a5", "38", "bf", "40", "a3", "9e", "81", "f3", "d7", "fb"},
        array<string, 16> {"7c", "e3", "39", "82", "9b", "2f", "ff", "87", "34", "8e", "43", "44", "c4", "de", "e9", "cb"},
        array<string, 16> {"54", "7b", "94", "32", "a6", "c2", "23", "3d", "ee", "4c", "95", "0b", "42", "fa", "c3", "4e"},
        array<string, 16> {"08", "2e", "a1", "66", "28", "d9", "24", "b2", "76", "5b", "a2", "49", "6d", "8b", "d1", "25"},
        array<string, 16> {"72", "f8", "f6", "64", "86", "68", "98", "16", "d4", "a4", "5c", "cc", "5d", "65", "b6", "92"},
        array<string, 16> {"6c", "70", "48", "50", "fd", "ed", "b9", "da", "5e", "15", "46", "57", "a7", "8d", "9d", "84"},
        array<string, 16> {"90", "d8", "ab", "00", "8c", "bc", "d3", "0a", "f7", "e4", "58", "05", "b8", "b3", "45", "06"},
        array<string, 16> {"d0", "2c", "1e", "8f", "ca", "3f", "0f", "02", "c1", "af", "bd", "03", "01", "13", "8a", "6b"},
        array<string, 16> {"3a", "91", "11", "41", "4f", "67", "dc", "ea", "97", "f2", "cf", "ce", "f0", "b4", "e6", "73"},
        array<string, 16> {"96", "ac", "74", "22", "e7", "ad", "35", "85", "e2", "f9", "37", "e8", "1c", "75", "df", "6e"},
        array<string, 16> {"47", "f1", "1a", "71", "1d", "29", "c5", "89", "6f", "b7", "62", "0e", "aa", "18", "be", "1b"},
        array<string, 16> {"fc", "56", "3e", "4b", "c6", "d2", "79", "20", "9a", "db", "c0", "fe", "78", "cd", "5a", "f4"},
        array<string, 16> {"1f", "dd", "a8", "33", "88", "07", "c7", "31", "b1", "12", "10", "59", "27", "80", "ec", "5f"},
        array<string, 16> {"60", "51", "7f", "a9", "19", "b5", "4a", "0d", "2d", "e5", "7a", "9f", "93", "c9", "9c", "ef"},
        array<string, 16> {"a0", "e0", "3b", "4d", "ae", "2a", "f5", "b0", "c8", "eb", "bb", "3c", "83", "53", "99", "61"},
        array<string, 16> {"17", "2b", "04", "7e", "ba", "77", "d6", "26", "e1", "69", "14", "63", "55", "21", "0c", "7d"}
    };
    const array<array<string, 4>, 4> c{
        array<string, 4> {"02", "03", "01", "01"},
        array<string, 4> {"01", "02", "03", "01"},
        array<string, 4> {"01", "01", "02", "03"},
        array<string, 4> {"03", "01", "01", "02"}
    };
    const array<array<string, 4>, 4> invC{
        array<string, 4> {"0e", "0b", "0d", "09"},
        array<string, 4> {"09", "0e", "0b", "0d"},
        array<string, 4> {"0d", "09", "0e", "0b"},
        array<string, 4> {"0b", "0d", "09", "0e"}
    };
    const array<array<string, 4>, 15> RCon{
        array<string, 4>{"00", "00", "00", "00"},
        array<string, 4>{"01", "00", "00", "00"},
        array<string, 4>{"02", "00", "00", "00"},
        array<string, 4>{"04", "00", "00", "00"},
        array<string, 4>{"08", "00", "00", "00"},
        array<string, 4>{"10", "00", "00", "00"},
        array<string, 4>{"20", "00", "00", "00"},
        array<string, 4>{"40", "00", "00", "00"},
        array<string, 4>{"80", "00", "00", "00"},
        array<string, 4>{"1b", "00", "00", "00"},
        array<string, 4>{"36", "00", "00", "00"},
        array<string, 4>{"77", "00", "00", "00"},
        array<string, 4>{"f5", "00", "00", "00"},
        array<string, 4>{"11", "00", "00", "00"},
        array<string, 4>{"39", "00", "00", "00"}
    };

    array<array<array<bitset<8>, 4>, 4>, 15> keySchedule{};
    array<array<bitset<8>, 4>, 4> state{};

    bitset<8> galMul(bitset<8> a1, bitset<8> b1) { // частично украдена
        unsigned char a{ static_cast<unsigned char>(a1.to_ulong()) };
        unsigned char b{ static_cast<unsigned char>(b1.to_ulong()) };
        gal8 res{ 0 };
        string strBin{};
        for (; b; b >>= 1) {
            if (b & 1)
                res ^= a;
            if (a & 0x80)
                a = (a << 1) ^ min_poly;
            else
                a <<= 1;
        }
        int i = 8; 
        return bitset<8>(res);
    }

    bitset<4> hexCharToBin(char c) {
        switch (toupper(c))
        {
        case '0': return bitset<4>{0b0000};
        case '1': return bitset<4>{0b0001};
        case '2': return bitset<4>{0b0010};
        case '3': return bitset<4>{0b0011};
        case '4': return bitset<4>{0b0100};
        case '5': return bitset<4>{0b0101};
        case '6': return bitset<4>{0b0110};
        case '7': return bitset<4>{0b0111};
        case '8': return bitset<4>{0b1000};
        case '9': return bitset<4>{0b1001};
        case 'A': return bitset<4>{0b1010};
        case 'B': return bitset<4>{0b1011};
        case 'C': return bitset<4>{0b1100};
        case 'D': return bitset<4>{0b1101};
        case 'E': return bitset<4>{0b1110};
        case 'F': return bitset<4>{0b1111};
        };
        return bitset<4>{0b0000};
    }

    string stringToHex(string str) {
        bitset<8> bin{};
        bitset<4> bin1{};
        bitset<4> bin2{};
        string newStr{};
        for (unsigned short i = 0; i < str.length() - 1; i++) {
            bin = bitset<8>(str[i]);
            bin1 = bitset<4>(bin.to_string().substr(0, 4));
            bin2 = bitset<4>(bin.to_string().substr(4, 4));
            stringstream hex1;
            hex1 << hex << uppercase << bin1.to_ulong();
            stringstream hex2;
            hex2 << hex << uppercase << bin2.to_ulong();
            newStr += hex1.str();
            newStr += hex2.str();
        }
        for (unsigned short i = static_cast<unsigned short>(newStr.length()) - 1; i > 0; i--) {
            if (newStr[i] == '0')
                newStr.erase(i, 1);
            else
                break;
        }
        return newStr;
    }

    bitset<8> concat(bitset<4> a, bitset<4> b) {
        const string strA{ a.to_string() };
        const string strB{ b.to_string() };
        return bitset<8>{ strA + strB };
    }

    bitset<8> hexToBin(string hexByte) {
        const bitset<4> hexByte1{ hexCharToBin(hexByte[0]) };
        const bitset<4> hexByte2{ hexCharToBin(hexByte[1]) };
        const bitset<8> binByte{ concat(hexByte1, hexByte2) };
        return binByte;
    }

    array<bitset<8>, 4> xorBytes(array<bitset<8>, 4> bytes1, array<bitset<8>, 4> bytes2) {
        array<bitset<8>, 4> newBytes{};
        for (unsigned short i = 0; i < 3; i++)
            newBytes[i] = bytes1[i] ^ bytes2[i];
        return newBytes;
    }

    void cycleShiftLeft(unsigned short rowNum) {
        array<bitset<8>, 4> row{ state[rowNum] };
        rotate(row.begin(), row.begin() + rowNum, row.end());
        state[rowNum] = row;
    }

    void cycleShiftRight(unsigned short rowNum) {
        array<bitset<8>, 4> row{ state[rowNum] };
        rotate(row.begin(), row.begin() + (4 - rowNum) , row.end());
        state[rowNum] = row;
    };

    string hexToString(string hex) {
        string str{};
        string subs{};
        unsigned char ch{};
        for (unsigned short i = 0; i < hex.length() / 2 - 1; i++) {
            subs = hex.substr(static_cast<_int64>(i) * 2, 2);
            ch = static_cast<unsigned char>(hexToBin(subs).to_ulong());
            //ch = static_cast<unsigned char>(hexToBin(str.substr(static_cast<_int64>(i) * 2, 2)).to_ulong());
            str.push_back(ch);
        }
        return str;
    };

    void subBytes() {
        bitset<8> byte{};
        for (unsigned short row = 0; row < 3; row++) {
            for (unsigned short column = 0; column < 3; column++) {
                byte = state[row][column];
                const unsigned int x = byte.test(7) * 8 + byte.test(6) * 4 + byte.test(5) * 2 + byte.test(4);
                const unsigned int y = byte.test(3) * 8 + byte.test(2) * 4 + byte.test(1) * 2 + byte.test(0);
                const string newHexByte{ SBox[x][y] };
                const bitset<8> newBinByte{ hexToBin(newHexByte) };
                state[row][column] = newBinByte;
            }
        }
    }

    void invSubBytes() {
        bitset<8> byte{};
        for (unsigned short row = 0; row < 3; row++) {
            for (unsigned short column = 0; column < 3; column++) {
                byte = state[row][column];
                const unsigned int x = byte.test(7) * 8 + byte.test(6) * 4 + byte.test(5) * 2 + byte.test(4);
                const unsigned int y = byte.test(3) * 8 + byte.test(2) * 4 + byte.test(1) * 2 + byte.test(0);
                const string newHexByte{ invSBox[x][y] };
                const bitset<8> newBinByte{ hexToBin(newHexByte) };
                state[row][column] = newBinByte;
            }
        }
    };

    unsigned short getNumOfBlocks(string text) {
        const double textLen = static_cast<double>(text.length());
        const unsigned int numberOfBlocks{ static_cast<unsigned int>(ceil(textLen / 16.0)) };
        return numberOfBlocks;
    }

    string getStrState(string text, unsigned int index) {
        string block = text.substr(static_cast<_int64>(index) * 16, static_cast<_int64>(index) * 16 + 16);
        unsigned _int64 len{ block.length() };
        if (block.length() < 16)
            for (unsigned short i = 0; i < (15 - len); i++)
                block.push_back(' ');
        return block;
    }

    void getState(string text, unsigned int index) {
        string block{ getStrState(text, index) };
        for (unsigned short i = 0; i < 15; i++)
            state[i % 4][static_cast<unsigned short>(floor(i / 4))] = bitset<8>(block[i]);
    }

    void shiftRows() {
        for (unsigned short i = 0; i < 3; i++)
            cycleShiftLeft(i);
    }

    void invShiftRows() {
        for (unsigned short i = 1; i < 3; i++)
            cycleShiftRight(i);
    };

    bitset<8> GFMultiplyBy2(bitset<8> byte) {
        return (byte << 1) ^ bitset<8>(0b0001'1011);
    }

    bitset<8> mixColumnsMultiply(array<bitset<8>, 4> byte, unsigned short index) {
        const array<string, 4> cBytes{ c[index] };
        array<bitset<8>, 4> intermidateRes{};
        for (unsigned short i = 0; i < 3; i++) {
            intermidateRes[i] = galMul(hexToBin(cBytes[i]), byte[i]);
        }
        const bitset<8> newByte{ intermidateRes[0] ^ intermidateRes[1] ^ intermidateRes[2] ^ intermidateRes[3] };
        return newByte;
    }

    bitset<8> invMixColumnsMultiply(array<bitset<8>, 4> byte, unsigned short index) {
        const array<string, 4> cBytes{ invC[index] };
        array<bitset<8>, 4> intermidateRes{};
        for (unsigned short i = 0; i < 3; i++) {
            intermidateRes[i] = galMul(hexToBin(cBytes[i]), byte[i]);
        }
        const bitset<8> newByte{ intermidateRes[0] ^ intermidateRes[1] ^ intermidateRes[2] ^ intermidateRes[3] };
        return newByte;
    }

    void mixColumns() {
        array<bitset<8>, 4> oldColumn{};
        for (unsigned short column = 0; column < 3; column++) {
            for (unsigned short row = 0; row < 3; row++)
                oldColumn[row] = state[row][column];
            for (unsigned short row = 0; row < 3; row++)
                state[row][column] = mixColumnsMultiply(oldColumn, row);
        }
    }

    void invMixColumns() {
        array<bitset<8>, 4> oldColumn{};
        for (unsigned short column = 0; column < 3; column++) {
            for (unsigned short row = 0; row < 3; row++)
                oldColumn[row] = state[row][column];
            for (unsigned short row = 0; row < 3; row++)
                state[row][column] = invMixColumnsMultiply(oldColumn, row);
        }

    }

    void initKey(string key) {
        keySchedule = array<array<array<bitset<8>, 4>, 4>, 15>{};
        string hexByte{};
        bitset<8> bin{};
        for (unsigned short i = 0; i < 2; i++) {
            for (unsigned short i1 = i * 4; i1 < i * 4 + 3; i1++) {
                for (unsigned short i2 = i1 * 4; i2 < i * 4 + 3; i2++) {
                    hexByte = key.substr(static_cast<_int64>(i2) * 2, 2);
                    bin = hexToBin(hexByte);
                    keySchedule[i][i1][i2 % 4] = bin;
                }
            }
        }
    }

    array<bitset<8>, 4> g(array<bitset<8>, 4> word, unsigned short round) {
        unsigned int x{};
        unsigned int y{};
        bitset<8> byte;
        array<bitset<8>, 4> RConArr{};
        for (unsigned short RC = 0; RC < 3; RC++)
            RConArr[RC] = hexToBin(RCon[static_cast<_int64>(round) + 1][RC]);
        rotate(word.begin(), word.begin() + 1, word.end());
        for (unsigned short byteIndex = 0; byteIndex < 3; byteIndex++) {
            byte = word[byteIndex];
            x = byte.test(7) * 8 + byte.test(6) * 4 + byte.test(5) * 2 + byte.test(4);
            y = byte.test(3) * 8 + byte.test(2) * 4 + byte.test(1) * 2 + byte.test(0);
            word[byteIndex] = hexToBin(SBox[x][y]);
        }
        word = xorBytes(word, RConArr);
        return word;
    }

    void expandKey(string key) {
        initKey(key);
        for (unsigned short round = 2; round < 14; round++) {
            for (unsigned short row = 0; row < 3; row++) {
                if (row == 0)
                    keySchedule[round][row] = xorBytes(keySchedule[round - 1][row], g(keySchedule[round - 1][3], round));
                else
                    keySchedule[round][row] = xorBytes(keySchedule[round - 1][row], g(keySchedule[round][row - 1], round));
            }
        }
    }

    void addRoundKey(unsigned short round) {
        for (unsigned short row = 0; row < 3; row++)
            state[row] = xorBytes(state[row], keySchedule[round][row]);
    }

    string stateToStr() {
        unsigned char stateChar{};
        string encryptedString{};
        for (unsigned short row = 0; row < 4; row++)
            for (unsigned short column = 0; column < 4; column++) {
                stateChar = static_cast<unsigned char>(state[row][column].to_ullong());
                encryptedString.push_back(stateChar);
            }
        return encryptedString;
    }

    string deleteSpaces(string block) {
        for (unsigned short i = 15; i > 0; i--) {
            if (block[i] == ' ')
                block.erase(i, 1);
            else
                break;
        }
        return block;
    }

    string encrypt(string key, string text) {
        string encryptedString{};
        expandKey(key);
        unsigned short numOfBlocks{ getNumOfBlocks(text) };
        for (unsigned short block = 0; block < numOfBlocks; block++) {
            getState(text, block);
            for (unsigned short round = 0; round < 13; round++) {
                subBytes();
                shiftRows();
                mixColumns();
                addRoundKey(round);
            }
            subBytes();
            shiftRows();
            addRoundKey(14);
            if (block == numOfBlocks - 1)
                encryptedString += deleteSpaces(stateToStr());
            else
                encryptedString += stateToStr();
        }
        return stringToHex(encryptedString);
    }

    string decrypt(string key, string text) {
        text = hexToString(text);
        string decryptedString{};
        array<array<bitset<8>, 4>, 4> state_d{};
        expandKey(key);
        unsigned short numOfBlocks{ getNumOfBlocks(text) };
        for (unsigned short block = 0; block < numOfBlocks; block++) {
            getState(text, block);
            addRoundKey(14);
            invShiftRows();
            invSubBytes();
            for (unsigned short round = 13; round > 0; round--) {
                state_d = state;
                addRoundKey(round);
                invMixColumns();
                invShiftRows();
                invSubBytes();
            }

            if (block == numOfBlocks - 1)
                decryptedString += deleteSpaces(stateToStr());
            else
                decryptedString += stateToStr();
        }
        return decryptedString;
    }
};
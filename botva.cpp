#include <iostream>
#include <cstdint>
#include <bitset>
#include <array>
#include <sstream>
#include <string>
#include <cmath>
#include "AES.h"
#include "DH.h"

using namespace std;
using namespace AES;
using namespace DH;

int main()
{/*
    string key;  //12yjslna4cn1ova9aqk7piqc60mk80cb
    cout << "Enter key, 32 symbols: ";
    cin >> key;
    key = stringToHex(key);
    string text;
    cout << "\nEnter text: ";
    ws(cin);
    getline(cin, text);
    string encryptedString{ encrypt(key, text) };
    cout << "The encrypted string is: " << encryptedString << '\n';
    string decryptedString{ decrypt(key, encryptedString) };
    cout << "The decrypted string is: " << decryptedString << '\n';
    */
    constexpr unsigned long long privateKey{ 9427224058058126247 };
    constexpr unsigned short p{ 439 };
    unsigned short publicKey1{ getPublicKey1(p, privateKey) };
    cout << getMasterKey(p, privateKey, publicKey1) << '\n';
    system("pause");
    return 1;
}

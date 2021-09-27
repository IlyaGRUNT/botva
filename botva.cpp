#include <iostream>
#include <cstdint>
#include <bitset>
#include <array>
#include <sstream>
#include <string>
#include "AES.h"
using namespace std;
using namespace AES;

int main()
{
    typedef uint8_t gal8;
   // bitset<8> a1{0b0101'1101};
    //bitset<8> b1{0b0000'0011};
    //string st{galMul(a1, b1).to_string()};
    //cout << st;
    string key { "12yjslna4cn1ova9aqk7piqc60mk80cb" };
    //cout << "Enter key, 32 symbols: ";
    //cin >> key;
    //key = hexToString(key);
    string text { "test" };
    //cout << "\nEnter text: ";
    //ws(cin);
    //getline(cin, text);
    string encryptedString{ encrypt(key, text) };
    cout << "The encrypted string is: " << encryptedString << '\n';
    string decryptedString{ decrypt(key, encryptedString) };
    cout << "The decrypted string is: " << decryptedString << '\n';
    system("pause");
    return 1;
}

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
    system("pause");
    return 1;
}

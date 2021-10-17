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
{
    srand(time(0));
    const array<unsigned short, 64> pArr{ getPArr() };
    const array<unsigned long long, 64> privateKeyArr1{ getPrivateKeyArr() };
    const array<unsigned long long, 64> privateKeyArr2{ getPrivateKeyArr() };
    const array<unsigned short, 64> publicKeyArr{ getPublicKeyArr(pArr, privateKeyArr1) };
    const string key{ getAESKey(pArr, privateKeyArr2, publicKeyArr) };
    string text;
    cout << "Enter text: ";
    getline(cin, text);
    string encryptedString{ encrypt(key, text) };
    cout << "The encrypted string is: " << encryptedString << '\n';
    string decryptedString{ decrypt(key, encryptedString) };
    cout << "The decrypted string is: " << decryptedString << '\n';
    system("pause");
    return 0;
}

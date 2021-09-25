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
    string key{};
    cout << "Enter key, 32 symbols: ";
    cin >> key;
    string text;
    cout << "\nEnter text: ";
    ws(cin);
    getline(cin, text);
    string encryptedString{ encrypt(stringToHex(key), text) };
    cout << encryptedString << '\n';
    system("pause");
    return 1;
}

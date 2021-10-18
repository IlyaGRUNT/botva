#include <iostream>
#include "AES.h"
#include "DH.h"
#include "client.h"

using namespace std;
using namespace AES;
using namespace DH;

int main()
{
    //srand(time(0));
    //const array<unsigned short, 64> pArr{ getPArr() };
    //const array<unsigned long long, 64> privateKeyArr1{ getPrivateKeyArr() };
    //const array<unsigned long long, 64> privateKeyArr2{ getPrivateKeyArr() };
    //const array<unsigned short, 64> publicKeyArr{ getPublicKeyArr(pArr, privateKeyArr1) };
    //const string key{ getAESKey(pArr, privateKeyArr2, publicKeyArr) };
    //string text;
    //cout << "Enter text: ";
    //getline(cin, text);
    //string encryptedString{ encrypt(key, text) };
    //cout << "The encrypted string is: " << encryptedString << '\n';
    //string decryptedString{ decrypt(key, encryptedString) };
    //cout << "The decrypted string is: " << decryptedString << '\n';
    //system("pause");
    //return 0;
	std::cout << "Enter your nickname: ";
	char nickname[256];
	std::cin >> nickname;

	SOCKET sock = createSocket();

	char dest[256];
	std::cout << "\nDestination: ";
	std::cin >> dest;

	char sd[4096] = "/shut_down";
	char chDest[4096] = "/ch_dest";

	std::thread listenThread(listenF);
	init(sock, nickname);
	while (true) {
		char msg[4096];
		std::cin >> msg;
		if (msg == sd) {
			shutdown(sock, nickname);
			listenThread.detach();
			break;
		}
		else if (msg == chDest) {
			std::cout << "\nDestination: ";
			std::cin >> dest;
		}
		else {
			std::array<unsigned short, 64> p_arr = getPArr();
			char p_set[512]{ *toSet(p_arr) };
			sendMessage(sock, nickname, dest, msg);
			std::cout << "\nmessage sent";
		}
	}

	system("pause");
	return 0;
}

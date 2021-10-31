#include <iostream>
#include "AES.h"
#include "DH.h"
#include "client.h"

using namespace std;
using namespace AES;
using namespace DH;
using namespace client;

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
	std::string nickname;
	std::cin >> nickname;
	const char* ch_nickname = nickname.c_str();

	WSADATA WSAdata;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &WSAdata);

	std::array<int, 2> ports = init(nickname);
	
	SOCKET sock = connectServ(ports[0]);

	char dest[256];
	std::cout << "\nDestination: ";
	std::cin >> dest;

	const char* sd = "/shut_down";
	const char* chDest = "/ch_dest";

	std::thread listenThread(listenF, ports[1]);
	while (true) {
		char* msg{};
		std::string str_msg;
		std::cout << "Enter your message: ";
		std::cin >> str_msg;
		msg = &str_msg[0];
		if (msg == sd) {
			shut_down(sock);
			break;
		}
		else if (msg == chDest) {
			std::cout << "Destination: ";
			std::cin >> dest;
		}
		else {
			bool error = sendMessage(sock, ch_nickname, dest, msg);
			if (!error)
				std::cout << "message sent\n";
		}
	}
	listenThread.detach();
	system("pause");
	return 0;
}

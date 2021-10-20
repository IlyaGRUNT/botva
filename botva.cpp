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
	char* ch_nickname{};
	ch_nickname = &nickname[0];

	WSADATA WSAdata;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &WSAdata);

	int port = init(ch_nickname);
	
	SOCKET sock = connectServ(port);

	char dest[256];
	std::cout << "\nDestination: ";
	std::cin >> dest;

	const char* sd = "/shut_down";
	const char* chDest = "/ch_dest";

	std::thread listenThread(listenF);
	while (true) {
		char* msg{};
		std::string str_msg;
		std::cin >> str_msg;
		msg = &str_msg[0];
		if (msg == sd) {
			shut_down(sock);
			listenThread.detach();
			break;
		}
		else if (msg == chDest) {
			std::cout << "\nDestination: ";
			std::cin >> dest;
		}
		else {
			std::array<unsigned short, 64> p_arr = getPArr();
			char* p_set{};
			to_set(p_arr, p_set);
			sendMessage(sock, ch_nickname, dest, msg);
			std::cout << "\nmessage sent";
		}
	}
	/*
	WSADATA WSAdata;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &WSAdata);

	std::string nickname{};
	std::cin >> nickname;
	char* ch_nickname{};
	ch_nickname = &nickname[0];
	int _port = init(ch_nickname);
	std::cout << _port;

	connectServ(_port);
	*/

	system("pause");
	return 0;
}

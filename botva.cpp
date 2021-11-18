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
	std::cout << "Enter your nickname: ";
	std::string nickname;
	std::cin >> nickname;
	const char* ch_nickname = nickname.c_str();

	WSADATA WSAdata;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &WSAdata);

	std::array<int, 2> ports = getPorts(nickname);
	
	SOCKET sock = connectToTCP(ports[0]);
	std::thread listenTh(listenThread, ports[1]);

	std::this_thread::sleep_for(std::chrono::seconds(1)); //condition variable from listenF

	char dest[256];
	std::cout << "\nDestination: ";
	std::cin >> dest;

	const char* sd = "/shut_down";
	const char* chDest = "/ch_dest";
	std::cout << "Ready for chat\n";

	while (true) {
		std::string str_msg;
		std::getline(std::cin >> std::ws, str_msg);
		const char* msg = str_msg.c_str();
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
	listenTh.detach();
	system("pause");
	return 0;
}

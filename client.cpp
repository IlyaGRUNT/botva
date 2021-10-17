#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <thread>
#include <string>

#define ip   "25.82.45.163"
#define port 53412

using namespace std::this_thread;
using namespace std::chrono;

wchar_t* toPCW(const std::string s)
{
	const char* charArray = &s[0];
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

char * deleteFromArray(char arr[256], unsigned short pos) {
	for (unsigned short i = pos; i < 255; i++)
	{
		arr[i] = arr[i + 1];
	}
	return arr;
}

void listenF() {
	WSADATA WSAdata;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &WSAdata) != 0) {
		std::cout << "EROR WSAStartup\n";
		exit(1);
	}

	SOCKADDR_IN sock_addr;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = htons(53412);
	sock_addr.sin_family = AF_INET;
	SOCKADDR_IN client_addr;

	int size = sizeof(sock_addr);

	while (true) {
		SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);

		u_long i_mode = 0;
		ioctlsocket(sListen, FIONBIO, &i_mode);
		std::cout << ioctlsocket(sListen, FIONBIO, &i_mode);

		bind(sListen, (SOCKADDR*)&sock_addr, size);

		listen(sListen, SOMAXCONN);

		SOCKET newinit;
		int client_size = sizeof(client_addr);
		newinit = accept(sListen, (SOCKADDR*)&client_addr, &client_size);

	}
}

SOCKET createSocket() {
	WSADATA WSAdata;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &WSAdata) != 0) {
		std::cout << "EROR WSAStartup\n";
		exit(1);
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, NULL);
	if (sock == INVALID_SOCKET)
		std::cout << "ERROR";

	return sock;
}

void connectServ(SOCKET sock) {
	SOCKADDR_IN sock_addr;
	InetPton(AF_INET, toPCW(ip), &sock_addr.sin_addr.s_addr);
	sock_addr.sin_port = htons(port);
	sock_addr.sin_family = AF_INET;
	int size{ sizeof(sock_addr) };
	connect(sock, (SOCKADDR*)&sock_addr, size);
}

void init(SOCKET sock, char nickname[256]) {
	connectServ(sock);
	char mode[256] = "init";
	send(sock, mode, sizeof(mode), NULL);
	send(sock, nickname, sizeof(nickname), NULL);
	closesocket(sock);
}

void shutdown(SOCKET sock, char nickname[256]) {
	connectServ(sock);
	char mode[256] = "shut_down";
	send(sock, mode, sizeof(mode), NULL);
	send(sock, nickname, sizeof(nickname), NULL);
	closesocket(sock);
}

void sendMessage(SOCKET sock, char nickname[256], char dest[256], char DH_set_to[512], char msg[4096]) {
	char error = 1;
	connectServ(sock);
	char mode[256] = "message";
	char respond[256];
	send(sock, mode, sizeof(mode), NULL);
	send(sock, nickname, sizeof(nickname), NULL);
	send(sock, dest, sizeof(dest), NULL);
	send(sock, DH_set_to, sizeof(DH_set_to), NULL);
	recv(sock, respond, sizeof(respond), NULL);
	if (respond[0] == 1) {
		char err_msg[256]{ *deleteFromArray(respond, 0) };
		std::cout << err_msg;
	}
	else {
		char DH_set_from[512]{ *deleteFromArray(respond, 0) };
		send(sock, msg, sizeof(msg), NULL);
		closesocket(sock);
	}
}

int main(int argc, char* argv[]) {
	std::cout << "Enter your nickname";
	char nickname[256];
	std::cin >> nickname;

	SOCKET sock = createSocket();

	std::thread listenThread(listenF);

	char dest[256];
	std::cout << "Destination: ";
	std::cin >> dest;

	while (true) {
		char msg[4096];
		std::cin >> msg;
		char sd[256] = "/shut_down";
		if (msg == sd) {
			shutdown(sock, nickname);
			listenThread.detach();
			break;
		}
		sendMessage(sock, nickname, dest, DH_set_to, msg);
	}

	system("pause");
	return 0;

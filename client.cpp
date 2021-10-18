#include "client.h"
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <thread>
#include <string>
#include <array>
#include <sstream>
#include "DH.h"
#include "AES.h"

#define ip   "25.33.129.217"
#define port 5341

using namespace std::this_thread;
using namespace std::chrono;
using namespace DH;
using namespace AES;

std::array<unsigned short, 64> fromSet(std::string s) {
	std::array<unsigned short, 64> result;
	std::stringstream s_stream(s);
	for (uint8_t i = 0; i < 64; i++) {
		std::string substr;
		getline(s_stream, substr, ';');
		char* ch_substr;
		ch_substr = &substr[0];
		result[i] = std::atoi(ch_substr);
	}
	return result;
}

wchar_t* toPCW(const std::string s)
{
	const char* charArray = &s[0];
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

char* toSet(std::array<unsigned short, 64> DH) {
	std::string DHtemp;
	for (uint8_t i = 0; i < 64; i++) {
		std::stringstream ss;
		ss << DH[i];
		std::string str = ss.str();
		DHtemp.append(str);
		if (i != 63)
			DHtemp.append(";");
	}
	char* DHstr;
	DHstr = &DHtemp[0];
	return DHstr;
}

char* deleteFromArray(char arr[256], unsigned short pos) {
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
		char nickname_to[256];
		char p_set[512];
		char DH_set_to[512];
		recv(newinit, nickname_to, sizeof(nickname_to), NULL);
		recv(newinit, p_set, sizeof(p_set), NULL);
		recv(newinit, DH_set_to, sizeof(DH_set_to), NULL);
		std::array<unsigned short, 64> p_arr = fromSet(p_set);
		std::array<unsigned short, 64> DH_str_to = fromSet(DH_set_to);
		std::array<unsigned long long, 64> private_keys = getPrivateKeyArr();
		std::array<unsigned short, 64> public_keys = getPublicKeyArr(p_arr, private_keys);
		char DH_set_from[512]{ *toSet(public_keys) };
		send(newinit, DH_set_from, sizeof(DH_set_from), NULL);
		std::string AES_key = getAESKey(p_arr, private_keys, DH_str_to);
		char ch_msg[4096];
		recv(newinit, ch_msg, sizeof(ch_msg), NULL);
		shutdown(newinit, 2);
		closesocket(newinit);
		std::string msg{ ch_msg };
		std::string decrypted_msg = decrypt(AES_key, msg);
		std::cout << '\n' << nickname_to << ": " << msg;
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
	std::cout << WSAGetLastError();
}

void init(SOCKET sock, char nickname[256]) {
	connectServ(sock);
	char mode[256] = "init";
	send(sock, mode, sizeof(mode), NULL);
	send(sock, nickname, sizeof(nickname), NULL);
	shutdown(sock, 2);
	closesocket(sock);
}

void shut_down(SOCKET sock, char nickname[256]) {
	connectServ(sock);
	char mode[256] = "shut_down";
	send(sock, mode, sizeof(mode), NULL);
	send(sock, nickname, sizeof(nickname), NULL);
	shutdown(sock, 2);
	closesocket(sock);
}

void sendMessage(SOCKET sock, char nickname[256], char dest[256], char msg[4096]) {
	char error = 1;
	std::array<unsigned short, 64>p_arr = getPArr();
	char p_set[512]{ *toSet(p_arr) };
	std::array<unsigned long long, 64>private_keys = getPrivateKeyArr();
	std::array<unsigned short, 64>public_keys = getPublicKeyArr(p_arr, private_keys);
	char DH_set_to[512]{ *toSet(public_keys) };
	connectServ(sock);
	char mode[256] = "message";
	char response[256];
	send(sock, mode, sizeof(mode), NULL);
	send(sock, nickname, sizeof(nickname), NULL);
	send(sock, dest, sizeof(dest), NULL);
	send(sock, p_set, sizeof(p_set), NULL);
	send(sock, DH_set_to, sizeof(DH_set_to), NULL);
	recv(sock, response, sizeof(response), NULL);
	if (response[0] == 1) {
		char err_msg[256]{ *deleteFromArray(response, 0) };
		std::cout << err_msg;
	}
	else {
		char DH_set_from[512]{ *deleteFromArray(response, 0) };
		std::array<unsigned short, 64> DH_from = fromSet(DH_set_from);
		std::string AES_key = getAESKey(p_arr, private_keys, DH_from);
		std::string encrypted_msg = encrypt(AES_key, msg);
		char* ch_encrypted_msg = &encrypted_msg[0];
		send(sock, ch_encrypted_msg, sizeof(ch_encrypted_msg), NULL);
		shutdown(sock, 2);
		closesocket(sock);
	}
}
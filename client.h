#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <array>
#include <thread>
#include <vector>

namespace client {
	std::vector<char*> from_ch(char* ch);
	void to_ch(std::vector<char*> vec, char* ch);
	std::array<unsigned short, 64> from_set(std::string s);
	wchar_t* toPCW(const std::string s);
	void to_set(std::array<unsigned short, 64> DH, char* dh_set);
	void deleteFromArray(char* arr, unsigned short pos);
	void listenF();
	SOCKET connectServ(int port);
	int init(char* nickname);
	void shut_down(SOCKET sock);
	void sendMessage(SOCKET sock, char* nickname, char* dest, char* msg);
}

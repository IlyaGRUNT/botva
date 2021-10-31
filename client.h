#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <array>
#include <thread>
#include <vector>

namespace client {
	std::string to_fixed_length(std::string str, unsigned short len);
	std::vector<std::string> from_ch(std::string str);
	std::string to_ch(std::vector<const char*> vec, unsigned short len);
	std::array<unsigned short, 64> from_set(std::string s);
	wchar_t* toPCW(const std::string s);
	std::string to_set(std::array<unsigned short, 64> DH);
	void deleteFromArray(char* arr, unsigned short pos);
	void listenF(int port);
	SOCKET connectServ(int port);
	std::array<int, 2> init(std::string nickname);
	void shut_down(SOCKET sock);
	bool sendMessage(SOCKET sock, const char* nickname, const char* dest, const char* msg);
}

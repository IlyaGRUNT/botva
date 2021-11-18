#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <array>
#include <thread>
#include <vector>

namespace client {
	std::string toFixedLength(std::string str, unsigned short len);
	std::vector<std::string> fromCh(std::string str);
	std::string to_ch(std::vector<const char*> vec, unsigned short len);
	std::array<unsigned short, 64> fromSet(std::string s);
	wchar_t* toPCW(const std::string s);
	std::string toSet(std::array<unsigned short, 64> DH);
	void deleteFromArray(char* arr, unsigned short pos);
	void listenThread(int port);
	std::array<int, 2> getPorts(std::string nickname);
	SOCKET connectToTCP(int port);
	void shut_down(SOCKET sock);
	bool sendMessage(SOCKET sock, const char* nickname, const char* dest, const char* msg);
}

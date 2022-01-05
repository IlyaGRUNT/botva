#pragma once

#include <string>
#include <array>
#include <thread>
#include <vector>

namespace client {
	std::string to_fixed_length(std::string str, unsigned short len);
	std::vector<std::string> from_ch(std::string str);
	std::string to_ch(std::vector<const char*> vec, unsigned short len);
	std::array<unsigned short, 64> from_set(std::string s);
	std::string to_set(std::array<unsigned short, 64> DH);
	void delete_from_array(char* arr, unsigned short pos);
	std::vector<std::string> get_AES_from(int sock);
	std::string get_AES_to(int sock, const char* dest);
	void recv_thread(int port, std::string ip);
	std::array<int, 2> get_ports(std::string nickname, std::string ip, int UDP_port);
	int connect_to_TCP(int port, std::string ip);
	void shut_down(int sock);
	bool send_message(int sock, const char* nickname, const char* dest, const char* msg);
}

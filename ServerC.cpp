#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <bitset>
#include <map>
#include <thread>
#include <chrono>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#define UDPport 781
#define TCPport 998

char delim{ ';' };
char to_fixed_symbol{ '@' };

std::map<std::string, std::string> users;

std::string to_fixed_length(std::string str, unsigned short len) {
	for (unsigned short i = str.length(); i < len; i++)
		str += to_fixed_symbol;
	return str;
}

std::vector<std::string> from_ch(std::string str) {
	std::string str_delim(1, delim);
	std::vector<std::string> res;
	boost::split(res, str, boost::is_any_of(str_delim), boost::token_compress_on);
	return res;
}

std::string to_ch(std::vector<const char*> vec, unsigned short len) {
	std::vector<char*> non_const_vec;
	for (unsigned short i = 0; i < vec.size(); i++) {
		const char* tmp = vec[i];
		char* non_const_tmp = const_cast<char*>(tmp);
		non_const_vec.push_back(non_const_tmp);
	}
	std::string str_delim(1, delim);
	std::stringstream ss;
	std::copy(non_const_vec.begin(), non_const_vec.end(), std::ostream_iterator<char*>(ss, str_delim.c_str()));
	std::string str = ss.str();
	str = to_fixed_length(str, len);
	return str;
}

wchar_t* toPCW(const std::string s) {
	const char* charArray = &s[0];
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

void TCPThread(int port, std::string nickname) {

	unsigned short error = 0;

	char enable = 1;
	//setsockopt(sListen, SOL_SOCKET, SO_DONTLINGER, &enable , sizeof(char));
	//setsockopt(sListen, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(char));

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN sListen_addr;
	sListen_addr.sin_addr.s_addr = INADDR_ANY;
	sListen_addr.sin_port = htons(port);
	sListen_addr.sin_family = AF_INET;
	int size = sizeof(sListen_addr);

	bind(sListen, (SOCKADDR*)&sListen_addr, size);

	listen(sListen, SOMAXCONN);

	SOCKADDR_IN client_addr;
	SOCKET newConnection;
	int client_size = sizeof(client_addr);
	std::cout << "\nTCP waiting for connection on port: " << port;
	newConnection = accept(sListen, (SOCKADDR*)&client_addr, &client_size);

	if (newConnection == 0) {
		std::cout << '\n' << port << ": failed to connect";
		error = 1;
	}
	else {
		//std::cout << LastError() << '\n';
		std::cout << '\n' << port << ": client connected";
		//std::cout << str << '\n';
		char ch_data[8192];

		int WSAError = WSAGetLastError();

		if (WSAError == 0) {
			while (true) {
				WSAError = WSAGetLastError();
				if (WSAError != 0)
					break;
				recv(newConnection, ch_data, sizeof(ch_data), NULL);
				std::string str_data = ch_data;
				if (ch_data == "/shutdown") {
					break;
				}
				else if (ch_data != "" || str_data.find(delim) != std::string::npos) {
					str_data.erase(remove(str_data.begin(), str_data.end(), to_fixed_symbol), str_data.end());
					std::vector<std::string> data = from_ch(str_data);

					std::string mode = data[0];

					if (mode == "message") {
						std::cout << '\n' << port << ": message";
						const char* char_to = data[1].c_str();
						const char* p_set = data[2].c_str();
						const char*  DH_set_to = data[3].c_str();

						std::string to{ char_to };
						if (users.find(nickname) == users.end()) {
							const char* msg = "er;Your connection is not initialized";
							std::cout << '\n' << port << ": " << msg;
							send(newConnection, msg, 8192, NULL);
						}
						else if (users.find(to) == users.end()) {
							const char* msg = "er;User, you are trying to send message to, is offline1";
							std::cout << '\n' << port << ": " << msg;
							send(newConnection, msg, 8192, NULL);
						}
						else {
							SOCKADDR_IN to_sock_addr;
							InetPton(AF_INET, toPCW(users[to]), &to_sock_addr.sin_addr.s_addr);
							to_sock_addr.sin_port = htons(TCPport);
							to_sock_addr.sin_family = AF_INET;
							int to_size = sizeof(to_sock_addr);

							SOCKET toConnection = socket(AF_INET, SOCK_STREAM, NULL);
							if (connect(toConnection, (SOCKADDR*)&to_sock_addr, to_size) != 0) {
								const char* msg = "er;User, you are trying to send message to, is offline2";
								std::cout << '\n' << port << ": " << msg;
								send(newConnection, msg, 8192, NULL);
							}
							else {
								std::vector<const char*> vec_request = { nickname.c_str(), p_set, DH_set_to};
								std::string str_request = to_ch(vec_request, 2048);

								send(toConnection, str_request.c_str(), 2048, NULL);

								char DH_set_from[512];
								char message[4096];

								recv(toConnection, DH_set_from, 512, NULL);

								std::string str_DH_set_from = DH_set_from;
								std::string ok = "ok;";
								std::string ok_str_DH_set_from = ok + str_DH_set_from;

								send(newConnection, ok_str_DH_set_from.c_str(), 512, NULL);
								recv(newConnection, message, 4096, NULL);
								send(toConnection, message, 4096, NULL);
							}
						}
					}
					else {
						error = 1;
						break;
					}
					if (WSAGetLastError() != 0)
						break;
				}
				else
					break;
			}
		}
		WSAError = WSAGetLastError();
		if (WSAError != 0)
			error = 1;
		std::cout << '\n' << port << ": thread finished with code " << error;

		users.erase(nickname);
		shutdown(newConnection, 2);
		closesocket(newConnection);
	}
}

int main(int argc, char* argv[]) {
	WSADATA WSAdata;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &WSAdata) != 0) {
		std::cout << "EROR WSAStartup\n";
		exit(1);
	}

	SOCKADDR_IN sock_addr;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = htons(UDPport);
	sock_addr.sin_family = AF_INET;

	int size = sizeof(sock_addr);

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	bind(sock, (SOCKADDR*)&sock_addr, size);
	while (true) {

		char raw_ch_nickname[256];
		SOCKADDR_IN client;
		int client_size = sizeof(client);
		
		std::cout << "\nUDP waiting for connections";

		recvfrom(sock, raw_ch_nickname, sizeof(raw_ch_nickname), NULL, (SOCKADDR*)&client, &client_size);

		std::string nickname = from_ch(raw_ch_nickname)[0];
		const char* ch_nickname = nickname.c_str();

		SOCKADDR_IN new_TCP_addr;
		new_TCP_addr.sin_addr.s_addr = INADDR_ANY;
		new_TCP_addr.sin_port = 0;
		new_TCP_addr.sin_family = AF_INET;
		int TCP_size = sizeof(new_TCP_addr);

		SOCKET TCP_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		bind(TCP_socket, (SOCKADDR*)&new_TCP_addr, TCP_size);

		getsockname(TCP_socket, (SOCKADDR*)&new_TCP_addr, &TCP_size);
		int port = new_TCP_addr.sin_port;
		std::string str_port;
		std::stringstream ss1;
		ss1 << port;
		ss1 >> str_port;
		const char* ch_port{};
		ch_port = &str_port[0];

		std::thread th(TCPThread, port, nickname);
		th.detach();

		/*int port = getsockname(TCP_socket, (SOCKADDR*)&new_TCP_addr, &TCP_size);*/

		sendto(sock, ch_port, 8, NULL, (SOCKADDR*)&client, sizeof(client));

		char ch_ip[256];
		inet_ntop(AF_INET, &client, ch_ip, sizeof(ch_ip));
		std::string ip = ch_ip;

		users[nickname] = ip;

		std::cout << "\nusers[" << nickname << "] = " << users[nickname];

		std::cout << "\nCreated TCP thread with port: " << new_TCP_addr.sin_port;
	}
}

// 0 - ok
// 1 - errorNULL,
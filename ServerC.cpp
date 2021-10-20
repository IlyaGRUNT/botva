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

#define UDPport 781

constexpr char delim = ';';

std::map<std::string, std::string> users;

std::vector<char*> from_ch(char* ch) {
	std::vector<char*> res;
	std::string str = ch;
	std::replace(str.begin(), str.end(), delim, ' ');
	std::stringstream ss;
	std::string tmp;
	while (ss >> tmp) {
		char* ch_tmp{};
		std::copy(tmp.begin(), tmp.end(), ch_tmp);
		res.push_back(ch_tmp);
	}
	return res;
}

wchar_t* toPCW(const std::string s)
{
	const char* charArray = &s[0];
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

int GetBlockingMode(int Sock)
{
	int iSize, iValOld, iValNew, retgso;
	iSize = sizeof(iValOld);
	retgso = getsockopt(Sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&iValOld, &iSize); // Save current timeout value
	if (retgso == SOCKET_ERROR) return (-1);
	iValNew = 1;
	retgso = setsockopt(Sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&iValNew, iSize); // Set new timeout to 1 ms
	if (retgso == SOCKET_ERROR) return (-1);

	// Ok! Try read 0 bytes.
	char buf[1]; // 1 - why not :)
	int retrcv = recv(Sock, buf, 0, MSG_OOB); // try read MSG_OOB
	int werr = WSAGetLastError();

	retgso = setsockopt(Sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&iValOld, iSize); // Set timeout to initial value
	if (retgso == SOCKET_ERROR) return (-2);

	if (werr == WSAENOTCONN) return (-1);
	if (werr == WSAEWOULDBLOCK) return 1;
	return 0;
}

void TCPThread(int port, std::string nickname) {
	constexpr char ok = 0;
	constexpr char error = 1;

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
	if (WSAGetLastError() != 0)
		std::cout << WSAGetLastError() << " listen\n";

	SOCKADDR_IN client_addr;
	SOCKET newConnection;
	int client_size = sizeof(client_addr);
	std::cout << "\nTCP waiting for connection on port: " << port;
	newConnection = accept(sListen, (SOCKADDR*)&client_addr, &client_size);

	if (WSAGetLastError() != 0)
		std::cout << WSAGetLastError() << " accept\n";


	if (newConnection == 0)
		std::cout << "\nERROR newConnection";
	else {
		//std::cout << LastError() << '\n';
		std::cout << "\nCLIENT CONNECTED";
		//std::cout << str << '\n';
		char ch_data[8096];
		while (true) {
			recv(newConnection, ch_data, sizeof(ch_data), NULL);
			std::string str_data = ch_data;
			if (ch_data == "/shutdown") {
				break;
			}
			else if (ch_data != "" || str_data.find(';') != std::string::npos) {
				std::vector<char*> data = from_ch(ch_data);

				char* mode = data[0];

				if (mode == "message") {
					std::cout << "\nmessage";
					char* char_from = data[1];
					char* char_to = data[2];
					char* p_set = data[3];
					char* DH_set_to = data[4];

					if (recv(newConnection, char_from, sizeof(char_from), NULL) == -1)
						break;
					std::string from{ char_from };
					std::string to{ char_to };
					if (users.find(from) == users.end()) {
						const char* msg = "Your connection is not initialized\n";
						send(newConnection, error + msg, sizeof(error + msg), NULL);
					}
					else if (users.find(to) == users.end()) {
						const char* msg = "User, you are trying to send message to, is offline\n";
						send(newConnection, error + msg, sizeof(error + msg), NULL);
					}
					else {
						SOCKADDR_IN to_sock_addr;
						InetPton(AF_INET, toPCW(users[to]), &to_sock_addr.sin_addr.s_addr);
						to_sock_addr.sin_port = htons(0);
						to_sock_addr.sin_family = AF_INET;
						int to_size = sizeof(to_sock_addr);

						SOCKET toConnection = socket(AF_INET, SOCK_STREAM, NULL);
						if (connect(toConnection, (SOCKADDR*)&to_sock_addr, to_size) != 0) {
							const char* msg = "User, you are trying to send message to, is offline\n";
							send(newConnection, error + msg, sizeof(error + msg), NULL);
						}
						else {
							char DH_set_from[512];
							char message[4096];
							
							recv(toConnection, DH_set_from, sizeof(DH_set_from), NULL);
							send(newConnection, ok + DH_set_from, sizeof(ok + DH_set_from), NULL);
							recv(newConnection, message, sizeof(message), NULL);
							send(toConnection, message, sizeof(message), NULL);
						}
					}
				}
			}
			users.erase(nickname);
			shutdown(newConnection, 2);
			closesocket(newConnection);
		}
	}
	std::terminate();
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

		char ch_nickname[256];
		SOCKADDR_IN client;
		int client_size = sizeof(client);
		
		std::cout << "UDP waiting for connections\n";

		recvfrom(sock, ch_nickname, sizeof(ch_nickname), NULL, (SOCKADDR*)&client, &client_size);

		std::string nickname = ch_nickname;

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
		char* ch_port{};
		ch_port = &str_port[0];

		std::thread th(TCPThread, port, nickname);
		th.detach();

		/*int port = getsockname(TCP_socket, (SOCKADDR*)&new_TCP_addr, &TCP_size);*/

		sendto(sock, ch_port, 8, NULL, (SOCKADDR*)&client, sizeof(client));

		char ch_ip[256];
		inet_ntop(AF_INET, &client, ch_ip, sizeof(ch_ip));
		std::string ip = ch_ip;

		users[nickname] = ip;

		std::cout << "Created TCP thread with port: " << new_TCP_addr.sin_port << '\n';
	}
}

// 0 - ok
// 1 - errorNULL,
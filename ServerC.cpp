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
#include <condition_variable>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#define UDPport 781

char delim{ ';' };
char to_fixed_symbol{ '@' };

struct user {
	std::unique_ptr<std::mutex> recv_mu;
	std::unique_ptr<std::condition_variable> recv_cv;
	std::unique_ptr<std::mutex> send_mu;
	std::unique_ptr<std::condition_variable> send_cv;
	std::string str_data;
	bool shut_down{};
};

std::map<std::string, user> users;

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

void TCPRecvThread(int port, SOCKET sock, std::string nickname) {
	std::mutex* self_mu = users[nickname].recv_mu.get();
	std::condition_variable* self_cv = users[nickname].recv_cv.get();

	std::unique_lock<std::mutex> self_lk(*self_mu);

	std::mutex* send_mu = users[nickname].send_mu.get();
	std::condition_variable* send_cv = users[nickname].send_cv.get();

	std::unique_lock<std::mutex> send_lk(*send_mu);

	unsigned short error = 0;

	char enable = 1;
	//setsockopt(sListen, SOL_SOCKET, SO_DONTLINGER, &enable , sizeof(char));
	//setsockopt(sListen, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(char));

	listen(sock, SOMAXCONN);

	SOCKADDR_IN client_addr;
	SOCKET newConnection;
	int client_size = sizeof(client_addr);
	std::cout << "\nTCP waiting for connection on port: " << port;
	newConnection = accept(sock, (SOCKADDR*)&client_addr, &client_size);

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
					users[nickname].shut_down = true;

					send_lk.unlock();
					send_cv->notify_one();
					send_lk.lock();

					closesocket(sock);
					closesocket(newConnection);

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
						std::vector<const char*> vec_request = { nickname.c_str(), p_set, DH_set_to};
						std::string str_request = to_ch(vec_request, 2048);

						users[nickname].str_data = str_request;
						send_lk.unlock();
						send_cv->notify_one();
						send_lk.lock();
						
						self_cv->wait(self_lk);

						std::string DH_str_from = users[nickname].str_data;
						const char* DH_set_from = DH_str_from.c_str();


						std::string str_DH_set_from = DH_set_from;
						std::string ok = "ok;";
						std::string ok_str_DH_set_from = ok + str_DH_set_from;

						send(newConnection, ok_str_DH_set_from.c_str(), 512, NULL);

						char message[4096];
						recv(newConnection, message, 4096, NULL);

						std::string str_msg = message;
						users[nickname].str_data = message;

						send_lk.unlock();
						send_cv->notify_one();
						send_lk.lock();
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

void TCPSendThread(int port, SOCKET sock1, std::string nickname) {

	listen(sock1, SOMAXCONN);

	SOCKADDR client_addr;
	int size = sizeof(client_addr);

	SOCKET sock = accept(sock1, &client_addr, &size);
	std::cout << '\n' << port << ": client connected to recv thread";

	std::mutex* self_mu = users[nickname].send_mu.get();
	std::condition_variable* self_cv = users[nickname].send_cv.get();

	std::unique_lock<std::mutex> self_lk(*self_mu);

	std::mutex* recv_mu = users[nickname].recv_mu.get();
	std::condition_variable* recv_cv = users[nickname].recv_cv.get();

	std::unique_lock<std::mutex> recv_lk(*recv_mu);

	while (true) {
		self_cv->wait(self_lk);

		if (users[nickname].shut_down) {
			users.erase(nickname);
			closesocket(sock);
			closesocket(sock1);
			break;

		}
		
		else {
			std::string str_data = users[nickname].str_data.c_str();
			send(sock, str_data.c_str(), str_data.length() + 1, NULL);

			char DH_set[512];
			recv(sock, DH_set, sizeof(DH_set), NULL);
			std::string str_DH_set = DH_set;
			users[nickname].str_data = DH_set;

			recv_lk.unlock();
			recv_cv->notify_one();
			recv_lk.lock();

			self_cv->wait(self_lk);

			if (users[nickname].shut_down) {
				users.erase(nickname);
				closesocket(sock);
				closesocket(sock1);
				break;
			}

			else {
				std::string msg = users[nickname].str_data;
				send(sock, msg.c_str(), msg.length() + 1, NULL);

				users[nickname].str_data = "";
			}
		}
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

		std::string str = "";
		users[nickname] = { std::make_unique<std::mutex>(), std::make_unique<std::condition_variable>(), std::make_unique<std::mutex>(), std::make_unique<std::condition_variable>(), str, false};

		SOCKADDR_IN recv_TCP_addr;
		recv_TCP_addr.sin_addr.s_addr = INADDR_ANY;
		recv_TCP_addr.sin_port = 0;
		recv_TCP_addr.sin_family = AF_INET;
		int recv_TCP_size = sizeof(recv_TCP_addr);

		SOCKET recv_TCP_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		bind(recv_TCP_socket, (SOCKADDR*)&recv_TCP_addr, recv_TCP_size);

		getsockname(recv_TCP_socket, (SOCKADDR*)&recv_TCP_addr, &recv_TCP_size);
		int recv_port = recv_TCP_addr.sin_port;
		std::string recv_str_port;
		std::stringstream ss1;
		ss1 << recv_port;
		ss1 >> recv_str_port;
		const char* recv_ch_port = recv_str_port.c_str();

		SOCKADDR_IN send_TCP_addr;
		send_TCP_addr.sin_addr.s_addr = INADDR_ANY;
		send_TCP_addr.sin_port = 0;
		send_TCP_addr.sin_family = AF_INET;
		int send_TCP_size = sizeof(send_TCP_addr);

		SOCKET send_TCP_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		bind(send_TCP_socket, (SOCKADDR*)&send_TCP_addr, send_TCP_size);

		getsockname(send_TCP_socket, (SOCKADDR*)&send_TCP_addr, &send_TCP_size);
		int send_port = send_TCP_addr.sin_port;
		std::string send_str_port;
		std::stringstream ss2;
		ss2 << send_port;
		ss2 >> send_str_port;
		const char* send_ch_port = send_str_port.c_str();

		std::thread recv_th(TCPRecvThread, recv_port, recv_TCP_socket, nickname);
		recv_th.detach();
		std::thread send_th(TCPRecvThread, send_port, send_TCP_socket, nickname);
		send_th.detach();

		std::string ports = to_ch({ recv_ch_port, send_ch_port }, 16);

		/*int port = getsockname(TCP_socket, (SOCKADDR*)&recv_TCP_addr, &TCP_size);*/

		sendto(sock, ports.c_str(), 16, NULL, (SOCKADDR*)&client, sizeof(client));

		char ch_ip[256];
		inet_ntop(AF_INET, &client, ch_ip, sizeof(ch_ip));
		std::string ip = ch_ip;

		std::cout << "\nCreated recv TCP thread with port: " << recv_TCP_addr.sin_port << "   send TCP thread with port: " << send_TCP_addr.sin_port;
	}
}

// 0 - ok
// 1 - errorNULL,
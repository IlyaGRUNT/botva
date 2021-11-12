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
	std::unique_ptr<std::mutex> mu;
	std::unique_ptr<std::condition_variable> cv;
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
	std::condition_variable* self_cv= users[nickname].cv.get();

	unsigned short error = 0;

	char enable = 1;
	//setsockopt(sListen, SOL_SOCKET, SO_DONTLINGER, &enable , sizeof(char));
	//setsockopt(sListen, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(char));

	listen(sock, SOMAXCONN);

	SOCKADDR client_addr;
	SOCKET newConnection;
	int client_size = sizeof(client_addr);
	std::cout << "\nTCP waiting for connection on port: " << port << ' ' << WSAGetLastError();
	newConnection = accept(sock, &client_addr, &client_size);

	if (newConnection == 0) {
		std::cout << '\n' << port << ": failed to connect";
		error = 1;
	}
	else {
		//std::cout << LastError() << '\n';
		std::cout << '\n' << port << ": client connected to recv thread";
		//std::cout << str << '\n';
		char ch_data[8192];

		int WSAError = WSAGetLastError();

		if (WSAError == 0) {
			while (true) {
				WSAError = WSAGetLastError();
				if (WSAError != 0)
					break;
				memset(ch_data, 0, 8192);
				std::wcout << '\n' << port << ": waiting for ch_data";
				recv(newConnection, ch_data, sizeof(ch_data), NULL);
				std::string str_data = ch_data;
				if (ch_data == "/shutdown") {
					users[nickname].shut_down = true;

					self_cv->notify_one();

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

						auto to_it = users.find(to);

						if (to_it == users.end()) {
							std::wcout << "\noffline";
							send(newConnection, to_fixed_length("er;User, you are trying to send message to, is offline;", 8192).c_str(), 8192, NULL);
						}

						else {
							std::wcout << "\nonline";
							std::vector<const char*> vec_request = { nickname.c_str(), p_set, DH_set_to };
							std::string str_request = to_ch(vec_request, 2048);

							to_it->second.str_data = str_request;

							std::mutex* to_mu = to_it->second.mu.get();
							std::condition_variable* to_cv = to_it->second.cv.get();
							std::unique_lock<std::mutex> to_lk(*to_mu);
							to_lk.unlock();

							to_cv->notify_one();

							to_lk.lock(); 
							to_cv->wait(to_lk);
							to_lk.unlock();

							std::string DH_str_from = to_it->second.str_data;
							const char* DH_set_from = DH_str_from.c_str();


							std::string str_DH_set_from = DH_set_from;
							std::string ok = "ok;";
							std::string ok_str_DH_set_from = ok + str_DH_set_from;

							send(newConnection, ok_str_DH_set_from.c_str(), 512, NULL);

							char message[4096];
							recv(newConnection, message, 4096, NULL);

							std::string str_msg = message;
							to_it->second.str_data = message;

							to_cv->notify_one();

							to_lk.lock();
							to_cv->wait(to_lk);
							to_lk.unlock();
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
		std::cout << '\n' << port << ": thread finished with code " << error << " WSAError: " << WSAGetLastError();

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
	std::cout << '\n' << port << ": client connected to send thread";

	std::mutex* mu = users[nickname].mu.get();
	std::condition_variable* cv = users[nickname].cv.get();

	std::unique_lock<std::mutex> lk(*mu);
	int WSAError = 0;

	while (true) {
		WSAError = WSAGetLastError();
		if (WSAError != 0)
			break;

		cv->wait(lk);
		lk.unlock();

		if (users[nickname].shut_down) {
			users.erase(nickname);
			closesocket(sock);
			closesocket(sock1);
			break;

		}
		
		else {
			std::string str_data = to_fixed_length(users[nickname].str_data.c_str(), 2048);

			send(sock, str_data.c_str(), str_data.length() + 1, NULL);

			WSAError = WSAGetLastError();
			if (WSAError != 0)
				break;

			char DH_set[512];
			recv(sock, DH_set, sizeof(DH_set), NULL);

			WSAError = WSAGetLastError();
			if (WSAError != 0)
				break;

			std::string str_DH_set = DH_set;
			users[nickname].str_data = DH_set;

			cv->notify_one();

			lk.lock();
			cv->wait(lk);
			lk.unlock();

			if (users[nickname].shut_down) {
				users.erase(nickname);
				closesocket(sock);
				closesocket(sock1);
				break;
			}
			else {
				std::string msg = users[nickname].str_data;
				send(sock, msg.c_str(), msg.length() + 1, NULL);
				WSAError = WSAGetLastError();
				if (WSAError != 0)
					break;

				users[nickname].str_data = "";
			}
			cv->notify_one();
			lk.lock();
		}
		
	}
	WSAError = WSAGetLastError();
	std::cout << '\n' << port << ": send thread finished with code " << (WSAError != 0) << " WSAError: " << WSAError;

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
		users[nickname] = { std::make_unique<std::mutex>(), std::make_unique<std::condition_variable>(), str, false};

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
		std::thread send_th(TCPSendThread, send_port, send_TCP_socket, nickname);
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
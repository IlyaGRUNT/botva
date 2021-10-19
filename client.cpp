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
#include <vector>
#include <boost/lexical_cast.hpp>
#include "DH.h"
#include "AES.h"

#define ip       "25.33.129.217"
#define UDP_port 5341

constexpr char delim{ ';' };
constexpr char set_delim{ '#' };

using namespace std::this_thread;
using namespace std::chrono;
using namespace DH;
using namespace AES;

namespace client {
	std::vector<char*> from_ch(char* ch) {
		std::vector<char*> res;
		std::string str = ch;
		std::replace(str.begin(), str.end(), delim, ' ');
		std::stringstream ss;
		std::string tmp;
		while (ss >> tmp) {
			char* ch_tmp;
			std::copy(tmp.begin(), tmp.end(), ch_tmp);
			res.push_back(ch_tmp);
		}
		return res;
	}

	void to_ch(std::vector<char*> vec, char* ch) {
		uint8_t vec_size = vec.size();
		std::string str_res;
		char* res;
		for (uint8_t i = 0; i < vec_size; i++) {
			str_res += vec[i];
			if (i != vec_size - 1)
				str_res += delim;
		}
		std::copy(str_res.begin(), str_res.end(), res);
		ch = res;
	}

	std::array<unsigned short, 64> from_set(std::string s) {
		std::array<unsigned short, 64> result;
		std::stringstream s_stream(s);
		for (uint8_t i = 0; i < 64; i++) {
			std::string substr;
			getline(s_stream, substr, set_delim);
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

	void to_set(std::array<unsigned short, 64> DH, char* dh_set) {
		std::string DHtemp;
		for (uint8_t i = 0; i < 64; i++) {
			std::stringstream ss;
			ss << DH[i];
			std::string str = ss.str();
			DHtemp.append(str);
			if (i != 63)
				DHtemp += set_delim;
		}
		char* DHstr;
		DHstr = &DHtemp[0];
		dh_set = DHstr;
	}

	void deleteFromArray(char* arr, unsigned short pos) {
		for (unsigned short i = pos; i < 255; i++)
		{
			arr[i] = arr[i + 1];
		}
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
			char* ch_respond1;
			recv(newinit, ch_respond1, sizeof(ch_respond1), NULL);
			std::vector<char*> respond1 = from_ch(ch_respond1);
			char* nickname_to = respond1[0];
			char* p_set = respond1[1];
			char* DH_set_to = respond1[2];

			std::array<unsigned short, 64> p_arr = from_set(p_set);

			std::array<unsigned short, 64> DH_str_to = from_set(DH_set_to);
			std::array<unsigned long long, 64> private_keys = getPrivateKeyArr();
			std::array<unsigned short, 64> public_keys = getPublicKeyArr(p_arr, private_keys);
			char* DH_set_from; 
			to_set(public_keys, DH_set_from);
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

	void connectServ(SOCKET sock, int port) {
		SOCKADDR_IN sock_addr;
		InetPton(AF_INET, toPCW(ip), &sock_addr.sin_addr.s_addr);
		sock_addr.sin_port = htons(port);
		sock_addr.sin_family = AF_INET;
		int size{ sizeof(sock_addr) };
		connect(sock, (SOCKADDR*)&sock_addr, size);
		std::cout << WSAGetLastError();
	}

	int init(char* nickname) {
		SOCKADDR tmp;
		int size = sizeof(tmp);

		SOCKADDR_IN serv_addr;
		InetPton(AF_INET, toPCW(ip), &serv_addr.sin_addr.s_addr);
		serv_addr.sin_port = htons(UDP_port);
		serv_addr.sin_family = AF_INET;

		SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		sendto(sock, nickname, sizeof(nickname), NULL, (SOCKADDR*)&serv_addr, sizeof(serv_addr));

		char ch_port[16];
		recvfrom(sock, ch_port, sizeof(ch_port), NULL, &tmp, &size);
		int _port = boost::lexical_cast<int>(ch_port);
		
		return _port;
	}

	void sendMessage(SOCKET sock, char* nickname, char* dest, char* msg) {
		char error = 1;
		std::array<unsigned short, 64>p_arr = getPArr();
		char* p_set;
		to_set(p_arr, p_set);
		std::array<unsigned long long, 64>private_keys = getPrivateKeyArr();
		std::array<unsigned short, 64>public_keys = getPublicKeyArr(p_arr, private_keys);
		char* DH_set_to;
		to_set(public_keys, DH_set_to);
		const char* mode = "message";
		char* response;
		//send(sock, mode, sizeof(mode), NULL);
		//send(sock, nickname, sizeof(nickname), NULL);
		//send(sock, dest, sizeof(dest), NULL);
		//send(sock, p_set, sizeof(p_set), NULL);
		//send(sock, DH_set_to, sizeof(DH_set_to), NULL);
		recv(sock, response, sizeof(response), NULL);
		if (response[0] == 1) {
			deleteFromArray(response, 0);
			std::cout << response;
		}
		else {
			deleteFromArray(response, 0);
			std::array<unsigned short, 64> DH_from = from_set(response);
			std::string AES_key = getAESKey(p_arr, private_keys, DH_from);
			std::string encrypted_msg = encrypt(AES_key, msg);
			char* ch_encrypted_msg = &encrypted_msg[0];
			send(sock, ch_encrypted_msg, sizeof(ch_encrypted_msg), NULL);
			shutdown(sock, 2);
			closesocket(sock);
		}
	}
}
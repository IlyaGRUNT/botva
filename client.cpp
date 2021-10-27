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
#include "DH.h"
#include "AES.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#define ip       "25.33.129.217"
#define UDP_port 781

char delim{ ';' };
char set_delim{ '#' };
char to_fixed_symbol{ '@' };

using namespace std::this_thread;
using namespace std::chrono;
using namespace DH;
using namespace AES;

namespace client {
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

	std::array<unsigned short, 64> from_set(std::string s) {
		std::array<unsigned short, 64> result;
		std::stringstream s_stream(s);
		for (uint8_t i = 0; i < 64; i++) {
			std::string substr;
			getline(s_stream, substr, set_delim);
			const char* ch_substr = substr.c_str();
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

	std::string to_set(std::array<unsigned short, 64> DH) {
		std::string str_delim(1, set_delim);
		std::stringstream ss;
		std::copy(DH.begin(), DH.end(), std::ostream_iterator<unsigned short>(ss, str_delim.c_str()));
		return ss.str();
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
		sock_addr.sin_port = htons(998);
		sock_addr.sin_family = AF_INET;
		SOCKADDR_IN client_addr;

		int size = sizeof(sock_addr);

		while (true) {
			SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);

			u_long i_mode = 0;
			ioctlsocket(sListen, FIONBIO, &i_mode);

			bind(sListen, (SOCKADDR*)&sock_addr, size);

			listen(sListen, SOMAXCONN);

			SOCKET newinit;
			int client_size = sizeof(client_addr);
			newinit = accept(sListen, (SOCKADDR*)&client_addr, &client_size);
			char ch_response1[512];
			recv(newinit, ch_response1, sizeof(ch_response1), NULL);
			if (ch_response1 && !ch_response1[0]) 
				break;

			std::string str_response1 = ch_response1;
			std::vector<std::string> str_vec_response1 = from_ch(str_response1);
			std::vector<char*> response1;
			for (uint8_t i = 0; i < str_response1.size(); i++) {
				std::string str_tmp = str_vec_response1[i];
				char* ch_tmp = &str_tmp[0];
				response1.push_back(ch_tmp);
			}

			char* nickname_to = response1[0];
			char* p_set = response1[1];
			char* DH_set_to = response1[2];

			std::array<unsigned short, 64> p_arr = from_set(p_set);

			std::array<unsigned short, 64> DH_str_to = from_set(DH_set_to);
			std::array<unsigned long long, 64> private_keys = getPrivateKeyArr();
			std::array<unsigned short, 64> public_keys = getPublicKeyArr(p_arr, private_keys);
			std::string DH_str_from = to_set(public_keys);
			char* DH_set_from = &DH_str_from[0];
			send(newinit, DH_set_from, sizeof(DH_set_from), NULL);
			std::string AES_key = getAESKey(p_arr, private_keys, DH_str_to);
			char ch_msg[4096];
			recv(newinit, ch_msg, sizeof(ch_msg), NULL);
			shutdown(newinit, 2);
			closesocket(newinit);
			std::string msg{ ch_msg };
			std::string decrypted_msg = decrypt(AES_key, msg);
		}
	}

	SOCKET connectServ(int port) {
		SOCKADDR_IN sock_addr;
		InetPton(AF_INET, toPCW(ip), &sock_addr.sin_addr.s_addr);
		sock_addr.sin_port = htons(port);
		sock_addr.sin_family = AF_INET;

		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		int size{ sizeof(sock_addr) };
		connect(sock, (SOCKADDR*)&sock_addr, size);
		return sock;
	}

	int init(std::string nickname) {
		std::string nickname1 = nickname + ';';

		SOCKADDR tmp;
		int size = sizeof(tmp);

		const char* ch_nickname = nickname1.c_str();

		SOCKADDR_IN serv_addr;
		InetPton(AF_INET, toPCW(ip), &serv_addr.sin_addr.s_addr);
		serv_addr.sin_port = htons(UDP_port);
		serv_addr.sin_family = AF_INET;

		SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		sendto(sock, ch_nickname, strlen(ch_nickname), NULL, (SOCKADDR*)&serv_addr, sizeof(serv_addr));

		char ch_port[7];
		recvfrom(sock, ch_port, 7, NULL, &tmp, &size);
		std::cout << "ch_port from recvfrom: " << ch_port << '\n';
		int _port = atoi(ch_port);
		
		return _port;
	}

	void shut_down(SOCKET sock) {
		const char* sd_cmd = "/shut down";
		send(sock, sd_cmd, sizeof(sd_cmd), NULL);
		shutdown(sock, 2);
		closesocket(sock);
	}

	bool sendMessage(SOCKET sock, const char* nickname, const char* dest, const char* msg) {
		bool error = false;
		std::array<unsigned short, 64>p_arr = getPArr();
		std::string p_str = to_set(p_arr);
		const char* p_set = p_str.c_str();
		std::array<unsigned long long, 64>private_keys = getPrivateKeyArr();
		std::array<unsigned short, 64>public_keys = getPublicKeyArr(p_arr, private_keys);
		std::string DH_str_to = to_set(public_keys);
		const char* DH_set_to = DH_str_to.c_str();
		std::string str_mode = "message";
		const char* mode = str_mode.c_str();
		std::vector<const char*> vec_request = { mode, nickname, dest, p_set, DH_set_to };
		std::string str_request = to_ch(vec_request, 8192);
		const char* request = str_request.c_str();
		std::cout << request;
		send(sock, request, 8192, NULL);
		char ch_response[8192];
		recv(sock, ch_response, sizeof(ch_response), NULL);
		std::string str_response = ch_response;

		std::vector<std::string> str_vec_response = from_ch(str_response);

		if (str_vec_response[0] == "er") {
			error = true;
			std::cout << "error: " << str_vec_response[1] << '\n';
		}
		else {
			std::vector<const char*> response;
			for (uint8_t i = 0; i < str_response.size(); i++) {
				std::string str_tmp = str_vec_response[i];
				const char* ch_tmp = str_tmp.c_str();
				response.push_back(ch_tmp);
			}


			std::array<unsigned short, 64> DH_from = from_set(response[1]);
			std::string AES_key = getAESKey(p_arr, private_keys, DH_from);
			std::string encrypted_msg = encrypt(AES_key, msg);
			const char* ch_encrypted_msg = encrypted_msg.c_str();
			send(sock, ch_encrypted_msg, 4096, NULL);
		}
		return error;
	}
}
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
#include <mutex>
#include <condition_variable>
#include "DH.h"
#include "AES.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#define ip       "25.33.129.217"
#define UDP_port 781

char delim{ ';' };
char set_delim{ '#' };
char to_fixed_symbol{ '@' };
char ws_replace{ '_' };

std::mutex mu;

using namespace std::this_thread;
using namespace std::chrono;
using namespace DH;
using namespace AES;

namespace client {

	std::string replace_ws_to(std::string s) {
		std::replace(s.begin(), s.end(), ' ', ws_replace);
		return s;
	}

	std::string replace_ws_from(std::string s) {
		std::replace(s.begin(), s.end(), ws_replace, ' ');
		return s;
	}

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

	wchar_t* toPCW(const std::string s) {
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
	

	void listenF(int port) {
		std::cout << "\nListenF thread started";
		WSADATA WSAdata;
		WORD DLLVersion = MAKEWORD(2, 1);
		if (WSAStartup(DLLVersion, &WSAdata) != 0) {
			std::cout << "EROR WSAStartup\n";
			exit(1);
		}

		SOCKADDR_IN sock_addr;
		InetPton(AF_INET, toPCW(ip), &sock_addr.sin_addr.s_addr);
		sock_addr.sin_port = port;
		sock_addr.sin_family = AF_INET;
		SOCKADDR_IN client_addr;

		int size = sizeof(sock_addr);

		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		connect(sock, (SOCKADDR*)&sock_addr, size);
		std::cout << "\nrecv thread connected server";
		int i = 0;
		while (true) {
			char ch_response1[2048];
			memset(ch_response1, 0, 2048);
			int is_recvd1 = recv(sock, ch_response1, sizeof(ch_response1), NULL);
			//std::cout << "\nfirst is_recvd1: " << is_recvd1;
			while (is_recvd1 == 1) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				is_recvd1 = recv(sock, ch_response1, sizeof(ch_response1), NULL);
			}

			std::string str_response1 = ch_response1;

			std::vector<std::string> str_vec_response1 = from_ch(str_response1);

			std::string nickname_from = str_vec_response1[0];
			const char* p_set = str_vec_response1[1].c_str();
			const char* DH_set_to = str_vec_response1[2].c_str();

			std::array<unsigned short, 64> p_arr = from_set(p_set);

			std::array<unsigned short, 64> DH_str_to = from_set(DH_set_to);
			std::array<unsigned long long, 64> private_keys = getPrivateKeyArr();
			std::array<unsigned short, 64> public_keys = getPublicKeyArr(p_arr, private_keys);
			std::string DH_str_from = to_set(public_keys);
			DH_str_from = to_fixed_length(DH_str_from, 512);
			const char* DH_set_from = DH_str_from.c_str();
			send(sock, DH_set_from, 512, NULL);
			std::string AES_key = getAESKey(p_arr, private_keys, DH_str_to);
			char ch_msg[4096];
			memset(ch_msg, 0, 4096);
			int is_recvd2 = recv(sock, ch_msg, sizeof(ch_msg), NULL);
			//std::cout << "\nfirst is_recvd2: " << is_recvd2;
			while (is_recvd2 == 1) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				is_recvd2 = recv(sock, ch_msg, sizeof(ch_msg), NULL);
			}
			std::cout << "\nfinal is_recvd2: " << is_recvd2;
			std::string str_msg = ch_msg;
			std::string msg = from_ch(str_msg)[0];
			std::cout << "\nencrypted msg: " << msg;
			std::string raw_decrypted_msg = decrypt(AES_key, msg);
			std::cout << "\nmsg: " << raw_decrypted_msg;

			std::string decrypted_msg = replace_ws_from(raw_decrypted_msg);

			std::cout << '\n' << nickname_from << ": " << decrypted_msg << '\n';
			i++;
		}
	}

	SOCKET connectServ(int port) {
		SOCKADDR_IN sock_addr;
		InetPton(AF_INET, toPCW(ip), &sock_addr.sin_addr.s_addr);
		sock_addr.sin_port = port;
		sock_addr.sin_family = AF_INET;

		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		int size{ sizeof(sock_addr) };
		connect(sock, (SOCKADDR*)&sock_addr, size);
		std::cout << "\nsend thread connected to server";
		return sock;
	}

	std::array<int, 2> init(std::string nickname) {
		std::array<int, 2> res;

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

		char ch_ports[16];
		recvfrom(sock, ch_ports, 16, NULL, &tmp, &size);
		std::string str_ports = ch_ports;
		std::vector<std::string> ports = from_ch(str_ports);
		
		res[0] = std::stoi(ports[0]);
		res[1] = std::stoi(ports[1]);

		return res;
	}

	void shut_down(SOCKET sock) {
		const char* sd_cmd = "/shut down";
		send(sock, sd_cmd, sizeof(sd_cmd), NULL);
		shutdown(sock, 2);
		closesocket(sock);
	}

	bool sendMessage(SOCKET sock, const char* nickname, const char* dest, const char* raw_msg) {
		//std::cout << "\nbeginning of sendMessage: " << WSAGetLastError();
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
		std::vector<const char*> vec_request = { mode, dest, p_set, DH_set_to };
		std::string str_request = to_ch(vec_request, 4094);
		const char* request = str_request.c_str();
		int tmp = send(sock, request, 8192, NULL);
		//std::cout << "\nrequest sent: " << tmp << " WSAError: " << WSAGetLastError();
		char ch_response[8192];
		recv(sock, ch_response, sizeof(ch_response), NULL);
		std::string str_response = ch_response;

		std::vector<std::string> str_vec_response = from_ch(str_response);

		if (str_vec_response[0] == "er") {
			error = true;
			std::cout << "error: " << str_vec_response[1] << '\n';
		}
		else {
			std::array<unsigned short, 64> DH_from = from_set(str_vec_response[1]);
			std::string AES_key = getAESKey(p_arr, private_keys, DH_from);
			std::string str_raw_msg = raw_msg;
			std::cout << "\nmsg before replace" << str_raw_msg;
			std::replace(str_raw_msg.begin(), str_raw_msg.end(), ' ', ws_replace);
			const char* msg = str_raw_msg.c_str();
			std::cout << "\nmsg after replace: " << msg;
			std::string encrypted_msg = to_fixed_length(encrypt(AES_key, msg) + ';', 4096);
			std::cout << "\nencrypted msg: " << encrypted_msg;
			unsigned short problem_char = static_cast<unsigned short>(encrypted_msg[6]);
			std::cout << "\nproblem is: " << problem_char;
			const char* ch_encrypted_msg = encrypted_msg.c_str();
			send(sock, ch_encrypted_msg, 2000, NULL);
		}
		return error;
	}
}
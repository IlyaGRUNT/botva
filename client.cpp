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

std::mutex mu;

using namespace std::this_thread;
using namespace std::chrono;
using namespace DH;
using namespace AES;

namespace client {
	std::string toFixedLength(std::string str, unsigned short len) {
		for (unsigned short i = str.length(); i < len; i++)
			str += to_fixed_symbol;
		return str;
	}

	std::vector<std::string> fromCh(std::string str) {
		std::string str_delim(1, delim);
		std::vector<std::string> res;
		boost::split(res, str, boost::is_any_of(str_delim), boost::token_compress_on);
		return res;
	}
	
	std::string toCh(std::vector<const char*> vec, unsigned short len) {
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
		str = toFixedLength(str, len);
		return str;
	}

	std::array<unsigned short, 64> fromSet(std::string s) {
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

	std::string toSet(std::array<unsigned short, 64> DH) {
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

	std::string getAESConnectionKeyTo(SOCKET sock, const char* dest) {
		bool error = false;
		std::array<unsigned short, 64>p_arr = getPArr();
		std::string p_str = toSet(p_arr);
		const char* p_set = p_str.c_str();
		std::array<unsigned long long, 64>private_keys = getPrivateKeyArr();
		std::array<unsigned short, 64>public_keys = getPublicKeyArr(p_arr, private_keys);
		std::string DH_str_to = toSet(public_keys);
		const char* DH_set_to = DH_str_to.c_str();
		std::string str_mode = "message";
		const char* mode = str_mode.c_str();
		std::vector<const char*> vec_request = { mode, dest, p_set, DH_set_to };
		std::string str_request = toCh(vec_request, 4094);
		const char* request = str_request.c_str();
		send(sock, request, 8192, NULL);
		char ch_response[8192];
		recv(sock, ch_response, sizeof(ch_response), NULL);
		std::string str_response = ch_response;

		std::vector<std::string> str_vec_response = fromCh(str_response);

		if (str_vec_response[0] == "er") {
			error = true;
			std::string error_msg =  "1;error: " + str_vec_response[1] + '\n';
			return error_msg;
		}
		else {
			std::array<unsigned short, 64> DH_from = fromSet(str_vec_response[1]);
			std::string AES_key = "0;" + getAESKey(p_arr, private_keys, DH_from);
			return AES_key;
		}
	}

	std::vector<std::string> getAESConnectionKeyFrom(SOCKET sock) {
		char ch_response1[2048];
		int is_recvd1 = recv(sock, ch_response1, sizeof(ch_response1), NULL);
		//std::cout << "\nfirst is_recvd1: " << is_recvd1;
		while (is_recvd1 == 1) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			is_recvd1 = recv(sock, ch_response1, sizeof(ch_response1), NULL);
		}

		std::string str_response1 = ch_response1;

		std::vector<std::string> str_vec_response1 = fromCh(str_response1);

		std::string nickname_from = str_vec_response1[0];
		const char* p_set = str_vec_response1[1].c_str();
		const char* DH_set_to = str_vec_response1[2].c_str();

		std::array<unsigned short, 64> p_arr = fromSet(p_set);

		std::array<unsigned short, 64> DH_str_to = fromSet(DH_set_to);
		std::array<unsigned long long, 64> private_keys = getPrivateKeyArr();
		std::array<unsigned short, 64> public_keys = getPublicKeyArr(p_arr, private_keys);
		std::string DH_str_from = toSet(public_keys);
		DH_str_from = toFixedLength(DH_str_from, 512);
		const char* DH_set_from = DH_str_from.c_str();
		send(sock, DH_set_from, 512, NULL);
		std::string AES_key = getAESKey(p_arr, private_keys, DH_str_to);
		std::vector<std::string> res = { nickname_from, AES_key };
		return res;
	}

	void listenThread(int port) {
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
			std::vector<std::string> recvd_data = getAESConnectionKeyFrom(sock);
			std::string nickname_from = recvd_data[0];
			std::string AES_key = recvd_data[1];
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
			std::string msg = fromCh(str_msg)[0];
			std::string decrypted_msg = decrypt(AES_key, msg);

			std::cout << '\n' << nickname_from << ": " << decrypted_msg << '\n';
			i++;
		}
	}

	std::array<int, 2> getPorts(std::string nickname) {
		SOCKADDR_IN server_addr;
		InetPton(AF_INET, toPCW(ip), &server_addr.sin_addr.s_addr);
		server_addr.sin_port = UDP_port;
		server_addr.sin_family = AF_INET;

		SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		nickname = nickname + ';';
		sendto(sock, nickname.c_str(), nickname.length(), NULL, (SOCKADDR*)&server_addr, sizeof(server_addr));

		char raw_ports[16];
		int server_addr_size = sizeof(server_addr);
		recvfrom(sock, raw_ports, sizeof(raw_ports), NULL, (SOCKADDR*)&server_addr, &server_addr_size);
		
		std::string str_ports = raw_ports;
		std::vector<std::string> vec_ports = fromCh(str_ports);
		std::array<int, 2> arr_ports{std::stoi(vec_ports[0]), std::stoi(vec_ports[1])};

		return arr_ports;
	}

	SOCKET connectToTCP(int port) {
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

	void shut_down(SOCKET sock) {
		const char* sd_cmd = "/shut down";
		send(sock, sd_cmd, sizeof(sd_cmd), NULL);
		shutdown(sock, 2);
		closesocket(sock);
	}

	bool sendMessage(SOCKET sock, const char* nickname, const char* dest, const char* msg) {
		bool error = false;
		auto AES_vec = fromCh(getAESConnectionKeyTo(sock, dest));
		if (AES_vec[0] == "1") {
			error = true;
			std::cout << AES_vec[1];
		}
		else {
			std::string AES_key = AES_vec[1];
			std::cout << "\nmsg after replace: " << msg;
			std::string encrypted_msg = toFixedLength(encrypt(AES_key, msg) + ';', 4096);
			std::cout << "\nencrypted msg: " << encrypted_msg;
			unsigned short problem_char = static_cast<unsigned short>(encrypted_msg[6]);
			std::cout << "\nproblem is: " << problem_char;
			const char* ch_encrypted_msg = encrypted_msg.c_str();
			send(sock, ch_encrypted_msg, 2000, NULL);
		}
		return error;
	}
}
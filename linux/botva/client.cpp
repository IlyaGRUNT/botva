#include "client.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <array>
#include <sstream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "DH.h"
#include "AES.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

char delim{ ';' };
char set_delim{ '#' };
char to_fixed_symbol{ '@' };

using namespace std::this_thread;
using namespace std::chrono;
using namespace DH;
using namespace AES;

void error(std::string msg) {
	perror(msg.c_str());
	exit(1);
}

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

	std::string to_set(std::array<unsigned short, 64> DH) {
		std::string str_delim(1, set_delim);
		std::stringstream ss;
		std::copy(DH.begin(), DH.end(), std::ostream_iterator<unsigned short>(ss, str_delim.c_str()));
		return ss.str();
	}

	void delete_from_array(char* arr, unsigned short pos) {
		for (unsigned short i = pos; i < 255; i++) {
			arr[i] = arr[i + 1];
		}
	}

	std::string get_AES_to(int sock, const char* dest) {
		int n{};
		bool is_error = false;
		std::array<unsigned short, 64>p_arr = get_p_arr();
		std::string p_str = to_set(p_arr);
		const char* p_set = p_str.c_str();
		std::array<unsigned long long, 64>private_keys = get_private_key_arr();
		std::array<unsigned short, 64>public_keys = get_public_key_arr(p_arr, private_keys);
		std::string DH_str_to = to_set(public_keys);
		const char* DH_set_to = DH_str_to.c_str();
		std::string str_mode = "message";
		const char* mode = str_mode.c_str();
		std::vector<const char*> vec_request = { mode, dest, p_set, DH_set_to };
		std::string str_request = to_ch(vec_request, 4094);
		const char* request = str_request.c_str();
		n = send(sock, request, 8192, 0);
		if (n == -1) {
			error("ERROR sending request getAESto");
		}
		char ch_response[8192];
		n = recv(sock, ch_response, sizeof(ch_response), 0);
		if (n == -1) {
			error("ERROR recieving data getAESto");
		}
		std::string str_response = ch_response;

		std::vector<std::string> str_vec_response = from_ch(str_response);

		if (str_vec_response[0] == "er") {
			is_error = true;
			std::string error_msg =  "1;error: " + str_vec_response[1] + '\n';
			return error_msg;
		}
		else {
			std::array<unsigned short, 64> DH_from = from_set(str_vec_response[1]);
			std::string AES_key = "0;" + get_AES_key(p_arr, private_keys, DH_from);
			return AES_key;
		}
	}

	std::vector<std::string> get_AES_from(int sock) {
		int n{};
		char ch_response1[2048];
		n = recv(sock, ch_response1, sizeof(ch_response1), 0);
		if (n == -1) {
			error("ERROR recieving getAESfrom");
		}

		std::string str_response1 = ch_response1;

		std::vector<std::string> str_vec_response1 = from_ch(str_response1);

		std::string nickname_from = str_vec_response1[0];
		const char* p_set = str_vec_response1[1].c_str();
		const char* DH_set_to = str_vec_response1[2].c_str();

		std::array<unsigned short, 64> p_arr = from_set(p_set);

		std::array<unsigned short, 64> DH_str_to = from_set(DH_set_to);
		std::array<unsigned long long, 64> private_keys = get_private_key_arr();
		std::array<unsigned short, 64> public_keys = get_public_key_arr(p_arr, private_keys);
		std::string DH_str_from = to_set(public_keys);
		DH_str_from = to_fixed_length(DH_str_from, 512);
		const char* DH_set_from = DH_str_from.c_str();
		n = send(sock, DH_set_from, 512, 0);
		if (n == -1) {
			error("ERROR sending DH_set getAESfrom");
		}
		std::string AES_key = get_AES_key(p_arr, private_keys, DH_str_to);
		std::vector<std::string> res = { nickname_from, AES_key };
		return res;
	}

	void recv_thread(int port, std::string ip) {
		int n{};
		std::cout << "\nrecv thread started" << std::flush;

		sockaddr_in sock_addr;
		sock_addr.sin_addr.s_addr = inet_addr(ip.c_str());
		sock_addr.sin_port = port;
		sock_addr.sin_family = AF_INET;

		int size = sizeof(sock_addr);

		int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == -1) {
			error("ERROR opening socket listen_thread");
		}
		n = connect(sock, (sockaddr*)&sock_addr, size);
		if (n == -1) {
			error("ERROR connecting server listen_thread");
		}
		std::cout << "\nrecv thread connected server" << std::flush;
		int i = 0;
		while (true) {
			std::vector<std::string> recvd_data = get_AES_from(sock);
			std::string nickname_from = recvd_data[0];
			std::string AES_key = recvd_data[1];
			char ch_msg[4096];
			memset(ch_msg, 0, 4096);
			n = recv(sock, ch_msg, sizeof(ch_msg), 0);
			if (n == -1) {
				error("ERROR recieving message listen_thread");
			}
			std::string str_msg = ch_msg;
			std::string msg = from_ch(str_msg)[0];
			std::string decrypted_msg = decrypt(AES_key, msg);

			std::cout << '\n' << nickname_from << ": " << decrypted_msg << '\n' << std::flush;
			i++;
		}
	}

	std::array<int, 2> get_ports(std::string nickname, std::string ip, int UDP_port) {
		int n{};
		sockaddr_in server_addr;
		server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
		server_addr.sin_port = htons(UDP_port);
		server_addr.sin_family = AF_INET;

		int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sock == -1) {
			error("ERROR opening socket get_ports");
		}

		nickname = nickname + ';';
		n = sendto(sock, nickname.c_str(), nickname.length(), 0, (sockaddr*)&server_addr, sizeof(server_addr));
		if (n == -1) {
			error("ERROR sending request for ports get_ports");
		}

		char raw_ports[16];
		socklen_t server_addr_size = sizeof(server_addr);
		n = recvfrom(sock, raw_ports, sizeof(raw_ports), 0, (sockaddr*)&server_addr, &server_addr_size);
		if (n == -1) {
			error("ERROR recieving ports get_ports");
		}
		std::string str_ports = raw_ports;
		std::vector<std::string> vec_ports = from_ch(str_ports);
		std::array<int, 2> arr_ports{std::stoi(vec_ports[0]), std::stoi(vec_ports[1])};

		return arr_ports;
	}

	int connect_to_TCP(int port, std::string ip) {
		int n{};
		sockaddr_in sock_addr;
		sock_addr.sin_addr.s_addr = inet_addr(ip.c_str());
		sock_addr.sin_port = port;
		sock_addr.sin_family = AF_INET;

		int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == -1) {
			error("ERROR opening socket connect_to_TCP");
		}

		socklen_t size = sizeof(sock_addr);
		n = connect(sock, (sockaddr*)&sock_addr, size);
		if (n == -1) {
			error("ERROR connecting to server connect_to_TCP");
		}
		std::cout << "\nsend thread connected to server" << std::flush;
		return sock;
	}

	void shut_down(int sock) {
		const char* sd_cmd = "/shut down";
		send(sock, sd_cmd, sizeof(sd_cmd), 0);
		shutdown(sock, 2);
		close(sock);
	}

	bool send_message(int sock, const char* nickname, const char* dest, const char* msg) {
		int n{};
		bool is_error = false;
		auto AES_vec = from_ch(get_AES_to(sock, dest));
		if (AES_vec[0] == "1") {
			is_error = true;
			std::cout << AES_vec[1] << std::flush;
		}
		else {
			std::string AES_key = AES_vec[1];
			std::string encrypted_msg = to_fixed_length(encrypt(AES_key, msg) + ';', 4096);
			const char* ch_encrypted_msg = encrypted_msg.c_str();
			n = send(sock, ch_encrypted_msg, encrypted_msg.length(), 0);
			if (n == -1) {
				error("ERROR sending message send_message");
			}
		}
		return is_error;
	}
}
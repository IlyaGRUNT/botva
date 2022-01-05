#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
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

std::map<std::string, user> g_users;

void error(std::string msg) {
	msg = '\n' + msg;
	perror(msg.c_str());
	std::terminate();
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

void TCP_recv_thread(int port, int sock, std::string nickname) {
	std::condition_variable* self_cv = g_users[nickname].cv.get();

	listen(sock, SOMAXCONN);

	sockaddr client_addr;
	int new_connection;
	socklen_t client_size = sizeof(client_addr);
	new_connection = accept(sock, &client_addr, &client_size);

	if (new_connection == -1) {
		error(nickname + " recv_thread: ERROR failed to connect to client");
	}
	else {
		std::cout << '\n' << nickname << " TCP_recv_hread: client connected to recv thread" << std::flush;
		char ch_data[8192];
		int n{};
		while (true) {
			memset(ch_data, 0, sizeof(ch_data));
			std::cout << '\n' << nickname << " TCP_recv_thread: waiting for ch_data" << std::flush;
			n = recv(new_connection, ch_data, sizeof(ch_data), 0);
			if (n <= 0) {
				error(nickname + " TCP_recv_thread: ERROR on recieving");
			}
			std::string str_data = ch_data;
			if (str_data == "/shutdown") {
				g_users[nickname].shut_down = true;

				self_cv->notify_one();

				close(sock);
				close(new_connection);

				break;
			}
			else if (str_data != "" || str_data.find(delim) != std::string::npos) {
				str_data.erase(remove(str_data.begin(), str_data.end(), to_fixed_symbol), str_data.end());
				std::vector<std::string> data = from_ch(str_data);

				std::string mode = data[0];

				if (mode == "message") {
					std::cout << '\n' << nickname << " TCP_recv_thread: message" << std::flush;
					const char* char_to = data[1].c_str();
					const char* p_set = data[2].c_str();
					const char*  DH_set_to = data[3].c_str();

					std::string to{ char_to };

					auto to_it = g_users.find(to);

					if (to_it == g_users.end()) {
						std::wcout << "\noffline";
						n = send(new_connection, to_fixed_length("er;User, you are trying to send message to, is offline;", 8192).c_str(), 8192, 0);
						if (n <= 0) {
							error(nickname + " TCP_recv_thread: ERROR on sending \"user offline\" message");
						}
					}

					else {
						std::cout << "\nonline" << std::flush;
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

						n = send(new_connection, ok_str_DH_set_from.c_str(), 512, 0);
						if (n <= 0) {
							error(nickname + " TCP_recv_thread: ERROR sending DH_set TCP_recv_thread");
						}

						char message[4096];
						n = recv(new_connection, message, 4096, 0);
						if (n <= 0) {
							error(nickname + " TCP_recv_thread: ERROR recieving message TCP_recv_thread");
						}

						std::string str_msg = message;
						to_it->second.str_data = message;

						to_cv->notify_one();

						to_lk.lock();
						to_cv->wait(to_lk);
						to_lk.unlock();
					}
				}
			}
		}
		g_users.erase(nickname);
		shutdown(new_connection, 2);
		close(new_connection);
	}
}

void TCP_send_thread(int port, int sock1, std::string nickname) {
	int n{};
	n = listen(sock1, SOMAXCONN);
	if (n == -1) {
		error(nickname + " TCP_send_thread: ERROR setting socket to listenning mode TCP_send_thread");
	}

	sockaddr client_addr;
	socklen_t size = sizeof(client_addr);

	int sock = accept(sock1, &client_addr, &size);
	if (sock == -1) {
		error(nickname + " TCP_send_thread: ERROR accepting connecton");
	}
	std::cout << '\n' << nickname << " TCP_send_thread: client connected to send thread" << std::flush;

	std::mutex* mu = g_users[nickname].mu.get();
	std::condition_variable* cv = g_users[nickname].cv.get();

	std::unique_lock<std::mutex> lk(*mu);
	while (true) {
		cv->wait(lk);
		lk.unlock();

		std::cout << '\n' << nickname << " TCP_send_thread: client connected to send thread" << std::flush;

		if (g_users[nickname].shut_down) {
			g_users.erase(nickname);
			close(sock);
			close(sock1);
			break;

		}
		
		else {
			std::string str_data = to_fixed_length(g_users[nickname].str_data.c_str(), 2048);

			n = send(sock, str_data.c_str(), str_data.length(), 0);
			if (n == -1) {
				error(nickname + " TCP_send_thread: ERROR on 1 sending");
			}

			char DH_set[512];
			n = recv(sock, DH_set, sizeof(DH_set), 0);
			if (n == -1) {
				error(nickname + " TCP_send_thread: ERROR on recieving DH_set");
			}

			std::string str_DH_set = DH_set;
			g_users[nickname].str_data = DH_set;

			cv->notify_one();

			lk.lock();
			cv->wait(lk);
			lk.unlock();

			if (g_users[nickname].shut_down) {
				g_users.erase(nickname);
				close(sock);
				close(sock1);
				break;
			}
			else {
				std::string msg = g_users[nickname].str_data;
				n = send(sock, msg.c_str(), msg.length() + 1, 0);
				if (n == -1) {
					error(nickname + " TCP_send_thread: ERROR on 2 sending");
				}

				g_users[nickname].str_data = "";
			}
			cv->notify_one();
			lk.lock();
		}
		
	}
}

int main(int argc, char* argv[]) {
	int n{};
	sockaddr_in sock_addr;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = htons(UDPport);
	sock_addr.sin_family = AF_INET;

	socklen_t size = sizeof(sock_addr);

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == -1) {
		error("main: ERROR opening socket");
	}

	n = bind(sock, (sockaddr*) &sock_addr, size);
	if (n == -1) {
		error("main: ERROR on binding socket");
	}

	while (true) {
		char raw_ch_nickname[256];
		sockaddr_in client;
		socklen_t client_size = sizeof(client);

		std::cout << "\nUDP waiting for connections" << std::flush;

		n = recvfrom(sock, raw_ch_nickname, sizeof(raw_ch_nickname), 0, (sockaddr*) &client, &client_size);
		if (n == -1) {
			error("main: ERROR on recvfrom");
		}

		std::string nickname = from_ch(raw_ch_nickname)[0];

		std::string str = "";
		g_users[nickname] = { std::make_unique<std::mutex>(), std::make_unique<std::condition_variable>(), str, false};

		sockaddr_in recv_TCP_addr;
		recv_TCP_addr.sin_addr.s_addr = INADDR_ANY;
		recv_TCP_addr.sin_port = 0;
		recv_TCP_addr.sin_family = AF_INET;
		socklen_t recv_TCP_size = sizeof(recv_TCP_addr);

		int recv_TCP_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		bind(recv_TCP_socket, (sockaddr*) &recv_TCP_addr, recv_TCP_size);

		getsockname(recv_TCP_socket, (sockaddr*) &recv_TCP_addr, &recv_TCP_size);
		int recv_port = recv_TCP_addr.sin_port;
		std::string recv_str_port;
		std::stringstream ss1;
		ss1 << recv_port;
		ss1 >> recv_str_port;
		const char* recv_ch_port = recv_str_port.c_str();

		sockaddr_in send_TCP_addr;
		send_TCP_addr.sin_addr.s_addr = INADDR_ANY;
		send_TCP_addr.sin_port = 0;
		send_TCP_addr.sin_family = AF_INET;
		socklen_t send_TCP_size = sizeof(send_TCP_addr);

		int send_TCP_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		bind(send_TCP_socket, (sockaddr*) &send_TCP_addr, send_TCP_size);

		getsockname(send_TCP_socket, (sockaddr*) &send_TCP_addr, &send_TCP_size);
		int send_port = send_TCP_addr.sin_port;
		std::string send_str_port;
		std::stringstream ss2;
		ss2 << send_port;
		ss2 >> send_str_port;
		const char* send_ch_port = send_str_port.c_str();

		std::thread recv_th(TCP_recv_thread, recv_port, recv_TCP_socket, nickname);
		recv_th.detach();
		std::thread send_th(TCP_send_thread, send_port, send_TCP_socket, nickname);
		send_th.detach();

		std::string ports = to_ch({ recv_ch_port, send_ch_port }, 16);

		sendto(sock, ports.c_str(), 16, 0, (sockaddr*) &client, sizeof(client));

		char ch_ip[256];
		inet_ntop(AF_INET, &client, ch_ip, sizeof(ch_ip));
		std::string ip = ch_ip;

		std::cout << "\nCreated recv TCP thread with port: " << recv_TCP_addr.sin_port << "   send TCP thread with port: " << send_TCP_addr.sin_port << std::flush;
	}
	return 0;
}
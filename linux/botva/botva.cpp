#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include "AES.h"
#include "DH.h"
#include "client.h"

using namespace std;
using namespace AES;
using namespace DH;
using namespace client;

int main()
{
	std::string ip, nickname;
	int UDP_port;
	std::string current_dir = std::filesystem::current_path().string();
	std::string cfg_path = current_dir + "/client.cfg";
	if (!std::filesystem::exists(cfg_path)) {
		std::cout << "cfg file have not been found, so one will be created now\nenter ip of server: " << std::flush;
		std::cin >> ip;
		std::cout << "enter UDP port number of server: " << std::flush;
		std::cin >> UDP_port;
		std::cout << "enter your nickname without whitespaces in it: " << std::flush;
		std::cin >> nickname;
		std::ofstream ocfg(cfg_path);
		std::string cfg_data = ip + '\n' + std::to_string(UDP_port) + '\n' + nickname;
		const char* c_cfg_data = cfg_data.c_str();
		ocfg.write(c_cfg_data, cfg_data.length());
	}
	else {
		std::string str_UDP_port;
		std::ifstream icfg(cfg_path);
		std::string line;
		std::array<std::string, 3> cfg_data;
		std::getline(icfg, ip);
		std::getline(icfg, str_UDP_port);
		std::getline(icfg, nickname);
		UDP_port = std::stoi(str_UDP_port);
	}
	const char* ch_nickname = nickname.c_str();

	std::array<int, 2> ports = get_ports(nickname, ip, UDP_port);
	
	int sock = connect_to_TCP(ports[0], ip);
	std::thread recv_th(recv_thread, ports[1], ip);

	std::this_thread::sleep_for(std::chrono::seconds(1)); //condition variable from listenF

	char dest[256];
	std::cout << "\nDestination: " << std::flush;
	std::cin >> dest;

	const char* sd = "/shut_down";
	const char* chDest = "/ch_dest";
	std::cout << "Ready for chat\n" << std::flush;

	while (true) {
		std::string str_msg;
		std::getline(std::cin >> std::ws, str_msg);
		const char* msg = str_msg.c_str();
		if (msg == sd) {
			shut_down(sock);
			break;
		}
		else if (msg == chDest) {
			std::cout << "Destination: " << std::flush;
			std::cin >> dest;
		}
		else {
			bool error = send_message(sock, ch_nickname, dest, msg);
			if (!error)
				std::cout << "message sent\n" << std::flush;
		}
	}
	recv_th.detach();
	return 0;
}

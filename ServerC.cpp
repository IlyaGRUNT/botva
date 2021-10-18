#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <bitset>
#include <map>

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

int main(int argc, char* argv[]) {
	std::map<std::string, std::string> users;
	char ok = 0;
	char error = 1;

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

	std::cout << "Server initialized\n";
	while (true) {
		SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
		if (sListen == INVALID_SOCKET) {
			std::cout << "ERROR sListen";
			return 1;
		}

		u_long i_mode = 0;
		ioctlsocket(sListen, FIONBIO, &i_mode);

		bind(sListen, (SOCKADDR*)&sock_addr, size);

		listen(sListen, SOMAXCONN);

		SOCKET newConnection;
		int client_size = sizeof(client_addr);
		std::cout << "Waiting for connection\n";
		newConnection = accept(sListen, (SOCKADDR*)&client_addr, &client_size);

		if (newConnection == 0)
			std::cout << "ERROR newConnection\n";
		else {
			//std::cout << WSAGetLastError() << '\n';
			std::cout << "CLIENT CONNECTED\n";
			//std::cout << str << '\n';
			char char_mode[256];
			recv(newConnection, char_mode, sizeof(char_mode), NULL);
			std::string mode{ char_mode };
			if (mode == "init") {
				std::cout << "init\n";
				struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
				struct in_addr ipAddr = pV4Addr->sin_addr;
				char char_ip[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &ipAddr, char_ip, INET_ADDRSTRLEN);
				char char_nickname[256];
				recv(newConnection, char_nickname, sizeof(char_nickname), NULL);
				std::string ip{ char_ip };
				std::string nickname{ char_nickname };
				users[nickname] = ip;
				shutdown(newConnection, 2);
				closesocket(newConnection);
			}
			if (mode == "shut_down") {
				std::cout << "shut down\n";
				char char_nickname[256];
				recv(newConnection, char_nickname, sizeof(char_nickname), NULL);
				std::string nickname{ char_nickname };
				users.erase(nickname);
				shutdown(newConnection, 2);
				closesocket(newConnection);
			}
			if (mode == "message") {
				std::cout << "message\n";
				char char_from[256];
				char char_to[256];
				char p_set[512];
				char DH_set_to[512];
				recv(newConnection, char_from, sizeof(char_from), NULL);
				std::string from{ char_from };
				recv(newConnection, char_to, sizeof(char_to), NULL);
				std::string to{ char_to };
				recv(newConnection, p_set, sizeof(p_set), NULL);
				recv(newConnection, DH_set_to, sizeof(DH_set_to), NULL);
				if (users.find(from) == users.end()) {
					char msg[255] = "Your connection is not initialized";
					send(newConnection, error + msg, sizeof(error + msg), NULL);
					closesocket(newConnection);
				}
				else if (users.find(to) == users.end()) {
					char msg[255] = "User, you are trying to send message to, is offline";
					send(newConnection, error + msg, sizeof(error + msg), NULL);
					closesocket(newConnection);
				}
				else {
					SOCKADDR_IN to_sock_addr;
					InetPton(AF_INET, toPCW(users[to]), &to_sock_addr.sin_addr.s_addr);
					to_sock_addr.sin_port = htons(2584);
					to_sock_addr.sin_family = AF_INET;
					int to_size = sizeof(to_sock_addr);

					SOCKET toConnection = socket(AF_INET, SOCK_STREAM, NULL);
					if (connect(toConnection, (SOCKADDR*)&to_sock_addr, to_size) != 0) {
						char msg[255] = "User, you are trying to send message to, is offline";
						send(newConnection, error + msg, sizeof(error + msg), NULL);
						closesocket(toConnection);
						closesocket(newConnection);
					}
					else {
						char DH_set_from[512];
						char message[4096];
						send(toConnection, p_set, sizeof(p_set), NULL);
						send(toConnection, DH_set_to, sizeof(DH_set_to), NULL);
						recv(toConnection, DH_set_from, sizeof(DH_set_from), NULL);
						send(newConnection, ok + DH_set_from, sizeof(ok + DH_set_from), NULL);
						recv(newConnection, message, sizeof(message), NULL);
						send(toConnection, message, sizeof(message), NULL);
						closesocket(toConnection);
						closesocket(newConnection);
					}
				}
			}
		}
	}

	system("pause");
	return 0;
}

// 0 - ok
// 1 - error
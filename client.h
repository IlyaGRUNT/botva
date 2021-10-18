#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <array>
#include <thread>

std::array<unsigned short, 64> fromSet(std::string s);
wchar_t* toPCW(const std::string s);
char* toSet(std::array<unsigned short, 64> DH);
char* deleteFromArray(char arr[256], unsigned short pos);
void listenF();
SOCKET createSocket();
void connectServ(SOCKET sock);
void init(SOCKET sock, char nickname[256]);
void shutdown(SOCKET sock, char nickname[256]);
void sendMessage(SOCKET sock, char nickname[256], char dest[256], char msg[4096]);

#pragma once
#define WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <iostream>
#include <detours.h>
#include <string>
#include <iomanip>
#include <vector>
#include <set>
using namespace std;

typedef int(WSAAPI* connectFunction)(SOCKET socket, const sockaddr* name, int namelen);
typedef int(WSAAPI* WSAConnectFunction)(SOCKET socket, const sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS);

extern connectFunction connectOriginal;
extern WSAConnectFunction WSAConnectOriginal;


extern bool fServer;
extern set<pair<DWORD, DWORD>> myAddresses; // address, mask
extern string targetAddress;

int WSAAPI connectReplaced(SOCKET socket, const sockaddr* name, int namelen);
int WSAAPI WSAConnectReplaced(SOCKET socket, const sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS);
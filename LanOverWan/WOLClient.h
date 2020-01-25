#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <map>
#include "WarcraftService.h"
#include "WOLMessages.h"
#include "WOLConstants.h"

using namespace std;

#if defined(_WIN32) && !defined(EFI32) && !defined(EFI64)
#define  WOL_API  __stdcall
#else
#define WOL_API
#endif

typedef void (WOL_API *AddServiceCallback) (void *client, WarcraftService service);
typedef void (WOL_API *UpdateServiceCallback) (void* client, string name, WarcraftServiceData* data);
typedef void (WOL_API *RemoveServiceCallback) (void* client, string name);
typedef void (WOL_API *DisconnectedCallback) (void* client);

class WOLClient
{
public:
	bool valid;
	bool connected = false;

	WOLClient(string address, string port);
	WOLClient(string address);
	WOLClient();
	~WOLClient();
	void connectToServer();
	void disconnect();
	string getPort();
	void setPort(string port);
	string getAddress();
	void setAddress(string port);
	void setAddCallback(AddServiceCallback callback);
	void setUpdateCallback(UpdateServiceCallback callback);
	void setRemoveCallback(RemoveServiceCallback callback);
	void setDisconnectCallback(DisconnectedCallback callback);
private:
	WSADATA wsaData;
	SOCKET connectSocket = INVALID_SOCKET;
	bool connecting = false;
	string port;
	string address;
	thread recieveThread;
	int sendData(char* bytes, int length);
	void recieve();
	bool enoughData(char* data, int len);
	AddServiceCallback addCallback = NULL;
	UpdateServiceCallback updateCallback = NULL;
	RemoveServiceCallback removeCallback = NULL;
	DisconnectedCallback disconnectCallback = NULL;
};


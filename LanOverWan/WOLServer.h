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

using namespace std;

const char DEFAULT_PORT[] = "58899";

struct Client {
	thread recvThread;
	SOCKET clientSocket = INVALID_SOCKET;
	bool defaultSettings;
	wstring appName;
	bool valid = true;
};

enum listeningState {
	NotListening,
	Listening,
	StartingUp,
	BindFailed,
	OtherError
};


class WOLServer
{
public:
	listeningState currentState = NotListening;
	bool valid;
	map<string, WarcraftSerivce> services;
public:
	WOLServer();																	//Done
	WOLServer(string port);															//Done
	~WOLServer();																	//Done
	bool startListening();															//Done
	void stopListening();															//Done
	string getPort();																//Done									
	void setPort(string port);														//Done
	bool addService(WarcraftSerivce service);
	bool updateService(WarcraftSerivce service);
	bool removeService(WarcraftSerivce service);
private:
	string port;
	vector<Client*> clients;
	thread listeningThread;
	WSADATA wsaData;
	SOCKET serverSocket = INVALID_SOCKET;
	void listenServer();													//Done
	int sendData(Client* client, char* bytes, int length);					//Done
	void recieve(Client* client);											//Done
	void sendToAll(char* bytes, int length);								//Done
	void initClient(SOCKET sock);											//Done
	void destroyClient(Client* client);										//Done
	char* generateAddMessage(WarcraftSerivce service, int& len);
	char* generateUpdateMessage(WarcraftSerivce service, int& len);
	char* generateRemoveMessage(WarcraftSerivce service, int& len);
	int addBytes(void* buff, void* bytes, uint16_t len);
};


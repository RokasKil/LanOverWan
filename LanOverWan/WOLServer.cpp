#include "WOLServer.h"


string WOLServer::getPort() {
	return port;
}

void WOLServer::setPort(string port) {
	this->port = port;

}


WOLServer::WOLServer(string port) {
	this->port = port;
	valid = false;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		//printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}
	valid = true;
}

WOLServer::WOLServer() : WOLServer(DEFAULT_PORT) {}

WOLServer::~WOLServer() {
	if (valid) {
		stopListening();
		WSACleanup();
		for (auto i = services.begin(); i != services.end(); i++) {
			delete[](*i).second.data->rdata;
			delete (*i).second.data;
		}
		services.clear();
	}

}

bool WOLServer::startListening() {
	if (!valid) {
		return false;
	}
	if (currentState != Listening) {
		currentState = StartingUp;
		cout << "Trying to start listenServer thread " << endl;
		listeningThread = thread(&WOLServer::listenServer, this);
		while (currentState == StartingUp) {
			Sleep(1);
		}
		if (currentState == Listening) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return true;
	}
}

void WOLServer::stopListening() {
	if (!valid || currentState != Listening) {
		return;
	}
	closesocket(serverSocket);
	for (int i = 0; i < clients.size(); i++) {
		if (clients[i]->clientSocket != INVALID_SOCKET) {
			shutdown(clients[i]->clientSocket, SD_BOTH);
			if (clients[i]->recvThread.joinable()) {
				clients[i]->recvThread.join();
			}
		}
		delete clients[i];
	}
	clients.clear();
	if (listeningThread.joinable()) {
		listeningThread.join();
	}
	currentState = NotListening;
	//delete//FINISH THIS
}

void WOLServer::destroyClient(Client* client) {

	if (client->clientSocket != INVALID_SOCKET) {
		shutdown(client->clientSocket, SD_BOTH);
	}
	if (client->recvThread.joinable()) {
		client->recvThread.join();
	}
	delete client;
}

void WOLServer::sendToAll(char* bytes, int length) {
	for (int i = 0; i < clients.size(); i++) {
		if (!clients[i]->valid) {
			destroyClient(clients[i]);
			clients.erase(clients.begin() + i);
			i--;
		}
	}
	for (int i = 0; i < clients.size(); i++) {
		int result = sendData(clients[i], bytes, length);
		if (result == -1) {
			clients[i]->valid = false;
			destroyClient(clients[i]);
			clients.erase(clients.begin() + i);
			i--;
		}
		
	}
}

int WOLServer::sendData(Client* client, char* bytes, int length) {
	if (client->valid && client->clientSocket != INVALID_SOCKET) {
		return send(client->clientSocket, bytes, length, 0);
	}
	else {
		return -1;
	}
}

void WOLServer::listenServer() {
	cout << "listenServer thread started " << endl;
	int iResult;
	addrinfo* result = NULL;
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	cout << "Getting address " << endl;
	// Resolve the server address and port
	iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
	if (iResult != 0) {
		//printf("getaddrinfo failed with error: %d\n", iResult);
		//WSACleanup();
		currentState = OtherError;
		return;
	}
	cout << "Creating socket " << endl;
	// Create a SOCKET for connecting to server
	serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (serverSocket == INVALID_SOCKET) {
		//printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		currentState = OtherError;
		return;
	}

	cout << "Binding Socket " << endl;
	// Setup the TCP listening socket
	// ::bind do this to use global namespace
	iResult = ::bind(serverSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		//printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(serverSocket);
		currentState = BindFailed;
		//WSACleanup();
		return;
	}

	cout << "freeaddrinfo " << endl;
	freeaddrinfo(result);

	cout << "Starting to listen" << endl;
	iResult = listen(serverSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		//printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		//WSACleanup();
		currentState = OtherError;
		return;
	}

	cout << "Success" << endl;
	currentState = Listening;

	cout << "Success listening on " << port << endl;
	// Accept a client socket
	SOCKET clientSocket;
	do {
		clientSocket = accept(serverSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			//printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(serverSocket);
			//WSACleanup();
			currentState = NotListening;
			//return 1;
		}
		else {
			initClient(clientSocket);
		}
	} while (clientSocket != INVALID_SOCKET);
	// No longer need server socket
	//closesocket(ListenSocket);
}

void WOLServer::recieve(Client* client) {
	if (client->clientSocket == INVALID_SOCKET) {
		client->valid = false;
		return;
	}
	char* buffer = new char[256];
	int iResult;
	int offset = 0;
	int newOffset;
	unsigned short length;
	int start = 0;
	do {// Clients don't send anything atm only recieve
		iResult = recv(client->clientSocket, buffer, 256, 0);
		if (iResult > 0) {}
		else if (iResult == 0) {}
		else {}
		//cout << ""recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);
	delete[] buffer;
	closesocket(client->clientSocket);
	client->clientSocket = INVALID_SOCKET;
	client->valid = false;
	return;
}

void WOLServer::initClient(SOCKET sock) {
	Client* c = new Client();
	c->clientSocket = sock;
	c->defaultSettings = true;
	c->recvThread = thread(&WOLServer::recieve, this, c);
	clients.push_back(c);

	for (auto i = services.begin(); i != services.end(); i++) {
		int length;
		char* message = generateAddMessage((*i).second, length);
		sendData(c, message, length);
		delete[] message;
	}
}


char* WOLServer::generateAddMessage(WarcraftSerivce service, int& len) {
	//															 host byte order
	//    A   name length      name     type length      type    port rrtype rdlen   rdata
	len = 1 + 2 + service.name.length() + 2 + service.type.length() + 2 + 2 + 2 + service.data->rdlen;
	int pos = 0;
	char* buff = new char[len];
	buff[pos++] = WOLMessage::Add;
	pos += addBytes(buff + pos, (void*)service.name.c_str(), service.name.length());
	pos += addBytes(buff + pos, (void*)service.type.c_str(), service.type.length());
	*((uint16_t*)(buff + pos)) = service.port;
	pos += 2;
	*((uint16_t*)(buff + pos)) = service.data->rrtype;
	pos += 2;
	pos += addBytes(buff + pos, (void*)service.data->rdata, service.data->rdlen);
	return buff;
	
}
char* WOLServer::generateUpdateMessage(WarcraftSerivce service, int& len) {
	//    U   namelen      name    rdlen   rdata
	len = 1 + 2 + service.name.length() + 2 + service.data->rdlen;
	int pos = 0;
	char* buff = new char[len];
	buff[pos++] = WOLMessage::Update;
	pos += addBytes(buff + pos, (void*)service.name.c_str(), service.name.length());
	pos += addBytes(buff + pos, (void*)service.data->rdata, service.data->rdlen);
	return buff;
}

char* WOLServer::generateRemoveMessage(WarcraftSerivce service, int& len) {
	//    R   namelen      name
	len = 1 + 2 + service.name.length();
	int pos = 0;
	char* buff = new char[len];
	buff[pos++] = WOLMessage::Remove;
	pos += addBytes(buff + pos, (void*)service.name.c_str(), service.name.length());
	return buff;
}

int WOLServer::addBytes(void* buff, void* bytes, uint16_t len) {
	*((uint16_t*)buff) = len;
	memcpy((char*)buff + 2, bytes, len);
	return len + 2;
}

bool WOLServer::addService(WarcraftSerivce service) {
	auto entry = services.find(service.name);
	if (entry == services.end()) {
		services[service.name] = service;
		services[service.name].data = new WarcraftServiceData();
		services[service.name].data->rdlen = service.data->rdlen;
		services[service.name].data->rrtype = service.data->rrtype;
		services[service.name].data->rdata = new char[service.data->rdlen];
		memcpy(services[service.name].data->rdata, service.data->rdata, service.data->rdlen);
		int length;
		char * message = generateAddMessage(services[service.name], length);
		sendToAll(message, length);
		delete[] message;
		return true;
	}
	else {
		return updateService(service);
	}
}
bool WOLServer::updateService(WarcraftSerivce service) {
	auto entry = services.find(service.name);
	if (entry == services.end()) {
		return false;
	}
	else {
		delete[] (*entry).second.data->rdata;
		delete (*entry).second.data;
		(*entry).second.data = new WarcraftServiceData();
		(*entry).second.data->rdlen = service.data->rdlen;
		(*entry).second.data->rrtype = service.data->rrtype;
		(*entry).second.data->rdata = new char[service.data->rdlen];
		memcpy((*entry).second.data->rdata, service.data->rdata, service.data->rdlen);
		int length;
		char* message = generateUpdateMessage((*entry).second, length);
		sendToAll(message, length);
		delete[] message;
		return true;
	}
}

bool WOLServer::removeService(WarcraftSerivce service) {
	auto entry = services.find(service.name);
	if (entry == services.end()) {
		return false;
	}
	else {
		int length;
		char* message = generateRemoveMessage((*entry).second, length);
		sendToAll(message, length);
		delete[] message;
		delete[](*entry).second.data->rdata;
		delete (*entry).second.data;
		services.erase(entry);
		return true;
	}
}
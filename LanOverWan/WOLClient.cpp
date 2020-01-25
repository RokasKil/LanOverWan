#include "WOLClient.h"

WOLClient::WOLClient(string address, string port) {
	this->address = address;
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
WOLClient::WOLClient(string address) : WOLClient("", DEFAULT_PORT) {};
WOLClient::WOLClient() : WOLClient("") {};

WOLClient::~WOLClient() {

	if (valid) {
		disconnect();
		//WSACleanup(wsaData)
	}
}

string WOLClient::getPort() {
	return port;
}

void WOLClient::setPort(string port) {
	this->port = port;
}

string WOLClient::getAddress() {
	return address;
}

void WOLClient::setAddress(string address) {
	this->address = address;
}

void WOLClient::setAddCallback(AddServiceCallback callback) {
	this->addCallback = callback;
}

void WOLClient::setUpdateCallback(UpdateServiceCallback callback) {
	this->updateCallback = callback;
}

void WOLClient::setRemoveCallback(RemoveServiceCallback callback) {
	this->removeCallback = callback;
}

void WOLClient::setDisconnectCallback(DisconnectedCallback callback) {
	this->disconnectCallback = callback;
}

void WOLClient::connectToServer() {
	if (!valid) {
		return;
	}
	connecting = true;
	recieveThread = thread(&WOLClient::recieve, this);
}

void WOLClient::disconnect() {
	if (connectSocket != INVALID_SOCKET) {
		connecting = false;
		shutdown(connectSocket, SD_BOTH);
		if (recieveThread.joinable()) {
			recieveThread.join();
		}
	}
}

int WOLClient::sendData(char* bytes, int length)
{
	int iResult = -1;
	if (connectSocket != INVALID_SOCKET && connected) {
		iResult = send(connectSocket, bytes, length, 0);
	}
	return iResult;
}

void WOLClient::recieve() {
	addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	int iResult;
	char* buffer = new char[1024];
	while (connecting) {
		if (connectSocket != INVALID_SOCKET) {
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
		}
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		iResult = getaddrinfo(address.c_str(), port.c_str(), &hints, &result);
		if (iResult != 0) {
			connecting = false;
			return;
		}
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (connectSocket == INVALID_SOCKET) {
				connecting = false;
				return;
			}
			cout << "Trying to connect " << endl;
			// Connect to server.
			iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(connectSocket);
				connectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}
		freeaddrinfo(result);
		if (connectSocket == INVALID_SOCKET) {
			//return ;
			Sleep(1000);
			continue;
		}
		int offset = 0;
		unsigned short length;
		int start = 0;
		connected = true;
		do {
			iResult = recv(connectSocket, buffer + offset, 1024 - offset, 0);
			if (iResult > 0) {
				offset += iResult;
				start = 0;
				while (enoughData(buffer + start, offset)) {
					uint16_t len;
					int pos = 0;
					switch (buffer[start + pos++]) {
					case WOLMessage::Add:
					{
						WarcraftService service;
						len = *((unsigned short*)(buffer + start + pos));
						//cout << "Name len: " << len << endl;
						pos += 2;
						service.name = string(buffer + start + pos, buffer + start + pos + len);
						pos += len;
						len = *((unsigned short*)(buffer + start + pos));
						//cout << "Type len: " << len << endl;
						pos += 2;
						service.type = string(buffer + start + pos, buffer + start + pos + len);
						pos += len;
						service.port = *((unsigned short*)(buffer + start + pos));
						//cout << "port: " << service.port << endl;
						pos += 2;
						service.data = new WarcraftServiceData();
						service.data->rrtype = *((unsigned short*)(buffer + start + pos));
						pos += 2;
						//cout << "rrtype: " << service.data->rrtype << endl;

						service.data->rdlen = *((unsigned short*)(buffer + start + pos));
						pos += 2;
						//cout << "rdlen: " << service.data->rdlen << endl;
						service.data->rdata = new char[service.data->rdlen];
						memcpy(service.data->rdata, buffer + start + pos, service.data->rdlen);
						pos += service.data->rdlen;

						if (addCallback != NULL) {
							addCallback(this, service);
						}
						break;
					}
					case WOLMessage::Update:
					{
						string name;
						WarcraftServiceData *data;
						len = *((unsigned short*)(buffer + start + pos));
						pos += 2;
						name = string(buffer + start + pos, buffer + start + pos + len);
						pos += len;
						data = new WarcraftServiceData();
						data->rrtype = -1;
						data->rdlen = *((unsigned short*)(buffer + start + pos));
						pos += 2;
						data->rdata = new char[data->rdlen];
						memcpy(data->rdata, buffer + start + pos, data->rdlen);
						pos += data->rdlen;
						if (updateCallback != NULL) {
							updateCallback(this, name, data);
						}
						break;
					}
					case WOLMessage::Remove:
					{
						string name;
						len = *((unsigned short*)(buffer + start + pos));
						pos += 2;
						name = string(buffer + start + pos, buffer + start + pos + len);
						pos += len;
						if (removeCallback != NULL) {
							removeCallback(this, name);
						}
						break;
					}
					default:
						iResult = -1;
						break;
					}
					if (iResult == -1) {
						break;
					}
					//cout << "parsed " << pos << "bytes" << endl;
					start += pos;
					offset -= pos;
					//cout << "new start " << start << endl;
					//cout << "new offset" << offset << endl;
				}
				if (iResult == -1) {
					break;
				}
				if (start != 0 && offset != 0) {
					memcpy(buffer, buffer + start, offset);

				}
				//printf("Bytes received: %d\n", iResult);
			}
			else if (iResult == 0) {}
			else {}
			//cout << ""recv failed with error: %d\n", WSAGetLastError());

		} while (iResult > 0);
		connected = false;
		if (disconnectCallback != NULL) {
			disconnectCallback(this);
		}
	}
	if (connectSocket != INVALID_SOCKET) {
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
	}
}

bool WOLClient::enoughData(char* data, int len) {
	int needs = 1;
	if (len <= 0) {
		return false;
	}
	switch (data[0]) {
	case WOLMessage::Add:
		if (len < needs + 2) {
			return false;
		}
		needs += *((short*)(data + needs)) + 2; // name + 2 for length
		if (len < needs + 2) {
			return false;
		}
		needs += *((short*)(data + needs)) + 2; // type + 2 for length
		needs +=  4; // port and rrtype
		if (len < needs + 2) {
			return false;
		}
		needs += *((short*)(data + needs)) + 2; // rdata + 2 for length
		return needs <= len;
	case WOLMessage::Update:
		if (len < needs + 2) {
			return false;
		}
		needs += *((short*)(data + needs)) + 2; // name + 2 for length
		if (len < needs + 2) {
			return false;
		}
		needs += *((short*)(data + needs)) + 2; // rdata + 2 for length
		return needs <= len;
	case WOLMessage::Remove:
		if (len < needs + 2) {
			return false;
		}
		needs += *((short*)(data + needs)) + 2; // name + 2 for length
		return needs <= len;
	}
	return true;
}

#include "functions.h"

connectFunction connectOriginal;
WSAConnectFunction WSAConnectOriginal;

bool fServer;
set<pair<DWORD, DWORD>> myAddresses; // address, mask;
string targetAddress;


int WINAPI connectReplaced(SOCKET socket, const sockaddr* name, int namelen) {
	//cout << "connect " << socket << " connecting" << std::endl;
	WSAPROTOCOL_INFOW info;
	int size = sizeof(info);
	if (getsockopt(socket, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&info, &size) == 0) {
		if ((info.iAddressFamily == AF_INET /*|| info.iAddressFamily == AF_INET6*/) &&
			info.iSocketType == SOCK_STREAM && info.iProtocol == IPPROTO_TCP) {
			string targetIp, targetPort;
			if (name->sa_family == AF_INET) {
				//cout << "Getting target" << endl;
				targetIp = inet_ntoa(((sockaddr_in*)name)->sin_addr);
				//cout << "Got ip " << targetIp << endl;
				targetPort = std::to_string(htons(((sockaddr_in*)name)->sin_port));
				//cout << "Got port " << targetPort << endl;
				//cout << "Target is " << targetIp << ":" << targetPort << endl;
				if (!fServer && find_if(myAddresses.begin(), myAddresses.end(), [name](const auto& arg) { return ((DWORD)((sockaddr_in*)name)->sin_addr.s_addr & arg.second) == (arg.first & arg.second); }) != myAddresses.end()) {
					//((sockaddr_in*)name)->sin_addr
					cout << "connect, target is " << targetIp << ":" << targetPort << endl;
					inet_pton(AF_INET, targetAddress.c_str(), &((sockaddr_in*)name)->sin_addr);
					targetIp = inet_ntoa(((sockaddr_in*)name)->sin_addr);
					cout << "Rerouting to " << targetIp << ":" << targetPort << endl;
				}
			}
		}
		else {
			cout << "Incorrect socket type: " << endl;
			cout << "iAddressFamily: " << info.iAddressFamily << endl;
			cout << "iSocketType: " << info.iSocketType << endl;
			cout << "iProtocol: " << info.iProtocol << endl;
		}
	}
	else {
		cout << "Failed to get socket options" << endl;
	}
	return connectOriginal(socket, name, namelen);
}

int WINAPI WSAConnectReplaced(SOCKET socket, const sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS) {
	//cout << "WSAConnect " << socket << " connecting" << std::endl;
	WSAPROTOCOL_INFOW info;
	int size = sizeof(info);
	if (getsockopt(socket, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&info, &size) == 0 &&
		(info.iAddressFamily == AF_INET /*|| info.iAddressFamily == AF_INET6*/) &&
		info.iSocketType == SOCK_STREAM && info.iProtocol == IPPROTO_TCP) {
		//cout << "Found a correct type of socket trying to route trough proxy" << endl;
		string targetIp, targetPort;
		if (name->sa_family == AF_INET) {
			//cout << "Getting target" << endl;
			targetIp = inet_ntoa(((sockaddr_in*)name)->sin_addr);
			//cout << "Got ip " << targetIp << endl;
			targetPort = std::to_string(htons(((sockaddr_in*)name)->sin_port));
			//cout << "Got port " << targetPort << endl;
			//cout << "Target is " << targetIp << ":" << targetPort << endl;
			if (!fServer && find_if(myAddresses.begin(), myAddresses.end(), [name](const auto& arg) { return ((DWORD)((sockaddr_in*)name)->sin_addr.s_addr & arg.second) == (arg.first & arg.second); }) != myAddresses.end()) {
				//((sockaddr_in*)name)->sin_addr
				cout << "WSAConnect, target is " << targetIp << ":" << targetPort << endl;
				inet_pton(AF_INET, targetAddress.c_str(), &((sockaddr_in*)name)->sin_addr);
				targetIp = inet_ntoa(((sockaddr_in*)name)->sin_addr);
				cout << "Rerouting to " << targetIp << ":" << targetPort << endl;
			}
		}
		else {
			cout << "Unsupported sa_family, how did you get here?" << endl;
			return WSAConnectOriginal(socket, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);
		}
	}
	return WSAConnectOriginal(socket, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);
}
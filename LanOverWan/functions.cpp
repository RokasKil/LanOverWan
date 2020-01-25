#include "functions.h"

struct socketInfo {
	SOCKET socket;
	bool isBlocking = false;
	bool WSAEventSelectUsed = false;
	WSAEVENT hEventObject;
	long lNetworkEvents;
	bool WSAAsyncSelectUsed = false;
	HWND hwnd;
	u_int wMsg;
	long lEvent;
	socketInfo(SOCKET socket) {
		this->socket = socket;
	}
};


ioctlsocketFunction ioctlsocketOriginal = NULL;
WSAIoctlFunction WSAIoctlOriginal = NULL;
WSAAsyncSelectFunction WSAAsyncSelectOriginal = NULL;
WSAEventSelectFunction WSAEventSelectOriginal = NULL;
CreateIoCompletionPortFunction CreateIoCompletionPortOriginal = NULL;
FreeLibraryFunction FreeLibraryOriginal = NULL;
CloseHandleFunction CloseHandleOriginal = NULL;

connectFunction connectOriginal;
WSAConnectFunction WSAConnectOriginal;
sendtoFunction sendtoOriginal;
recvfromFunction recvfromOriginal;
WSASendToFunction WSASendToOriginal;
WSARecvFromFunction WSARecvFromOriginal;
closesocketFunction closesocketOriginal;
WSAGetOverlappedResultFunction WSAGetOverlappedResultOriginal;
WSAWaitForMultipleEventsFunction WSAWaitForMultipleEventsOriginal;
DispatchMessageFunction DispatchMessageOriginal;
GetOverlappedResultFunction GetOverlappedResultOriginal;
GetOverlappedResultExFunction GetOverlappedResultExOriginal;
GetQueuedCompletionStatusFunction GetQueuedCompletionStatusOriginal;
GetQueuedCompletionStatusExFunction GetQueuedCompletionStatusExOriginal;
WSAConnectByNameWFunction WSAConnectByNameWOriginal;
WSAConnectByNameAFunction WSAConnectByNameAOriginal;
WSAConnectByListFunction WSAConnectByListOriginal;
LPFN_CONNECTEX ConnectExOriginal = NULL;
LPFN_CONNECTEX ConnectExReturned = NULL;
sendFunction sendOriginal;
recvFunction recvOriginal;
IsDebuggerPresentFunction IsDebuggerPresentOriginal;
bool fServer;
set<string> myAddresses;
string targetAddress;

int WINAPI WSAIoctlReplaced(SOCKET s, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	GUID connectExGUID = WSAID_CONNECTEX;
	int result = WSAIoctlOriginal(s, dwIoControlCode, lpvInBuffer, cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine);
    if (result == 0 && dwIoControlCode == SIO_GET_EXTENSION_FUNCTION_POINTER && *((GUID*)lpvInBuffer) == connectExGUID) {
		cout << "Got the ConnectEx function" << endl;
		cout << "It's " << *(LPFN_CONNECTEX*)lpvOutBuffer << endl;
		cout << "Replacement is  " << ConnectExReplaced << endl;
		if (ConnectExReturned != NULL && ConnectExReturned != *(LPFN_CONNECTEX*)lpvOutBuffer) {
			cout << "Got a different one this time??" << endl;
			cout << "Trying to hook it too might break, forget stuff" << endl;
		}
		if ((ConnectExReturned != NULL && ConnectExReturned != *(LPFN_CONNECTEX*)lpvOutBuffer) || ConnectExReturned == NULL) {
			ConnectExReturned = *(LPFN_CONNECTEX*)lpvOutBuffer;
			ConnectExOriginal = *(LPFN_CONNECTEX*)lpvOutBuffer;
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach((PVOID*)(&ConnectExOriginal), ConnectExReplaced);
			DetourTransactionCommit();
		}
	}
	return result;
}


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
				if (!fServer && find(myAddresses.begin(), myAddresses.end(), targetIp) != myAddresses.end()) {
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
			if (fServer && myAddresses.find(targetIp) != myAddresses.end()) {
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

BOOL WINAPI WSAConnectByNameWReplaced(SOCKET s, LPWSTR nodename, LPWSTR servicename, LPDWORD LocalAddressLength, LPSOCKADDR LocalAddress, LPDWORD RemoteAddressLength, LPSOCKADDR RemoteAddress, const timeval* timeout, LPWSAOVERLAPPED Reserved) {
	cout << "WSAConnectByNameW detected " << s << " " << nodename << " " << servicename << endl;
	return WSAConnectByNameWOriginal(s, nodename, servicename, LocalAddressLength, LocalAddress, RemoteAddressLength, RemoteAddress, timeout, Reserved);
}

BOOL WINAPI WSAConnectByNameAReplaced(SOCKET s, LPCSTR nodename, LPCSTR servicename, LPDWORD LocalAddressLength, LPSOCKADDR LocalAddress, LPDWORD RemoteAddressLength, LPSOCKADDR RemoteAddress, const timeval* timeout, LPWSAOVERLAPPED Reserved) {
	cout << "WSAConnectByNameA detected " << s << " " << nodename << " " << servicename << endl;
	return WSAConnectByNameAOriginal(s, nodename, servicename, LocalAddressLength, LocalAddress, RemoteAddressLength, RemoteAddress, timeout, Reserved);
}

BOOL WINAPI WSAConnectByListReplaced(SOCKET s, PSOCKET_ADDRESS_LIST SocketAddress, LPDWORD LocalAddressLength, LPSOCKADDR LocalAddress, LPDWORD RemoteAddressLength, LPSOCKADDR RemoteAddress, const timeval* timeout, LPWSAOVERLAPPED Reserved) {
	cout << "WSAConnectByList detected " << s << endl;
	return WSAConnectByListOriginal(s, SocketAddress, LocalAddressLength, LocalAddress, RemoteAddressLength, RemoteAddress, timeout, Reserved);
}

HANDLE CompletionPortEx;
ULONG_PTR lpCompletionKeyEx;
LPOVERLAPPED lpOverlappedEx;

BOOL WSAAPI ConnectExReplaced(SOCKET s, const sockaddr* name, int namelen, PVOID lpSendBuffer, DWORD dwSendDataLength, LPDWORD lpdwBytesSent, LPOVERLAPPED lpOverlapped) {

	cout << "ConnectEx detected " << s << " " << lpOverlapped << endl;

	WSAPROTOCOL_INFOW info;
	int size = sizeof(info);
	if (getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&info, &size) == 0) {
		if ((info.iAddressFamily == AF_INET /*|| info.iAddressFamily == AF_INET6*/) &&
			info.iSocketType == SOCK_STREAM && info.iProtocol == IPPROTO_TCP) {
			cout << "Found a correct type of socket trying to route trough proxy" << endl;
			string targetIp, targetPort;
			if (name->sa_family == AF_INET) {
				cout << "Getting target" << endl;
				targetIp = inet_ntoa(((sockaddr_in*)name)->sin_addr);
				cout << "Got ip " << targetIp << endl;
				targetPort = std::to_string(htons(((sockaddr_in*)name)->sin_port));
				cout << "Got port " << targetPort << endl;
				cout << "Target is " << targetIp << ":" << targetPort << endl;
			}
			else {
				cout << "Unsupported sa_family, how did you get here?" << endl;
				return ConnectExOriginal(s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);
			}
		}
		else {
			cout << "Incorrect socket type (ConnectEx): " << endl;
			cout << "iAddressFamily: " << info.iAddressFamily << endl;
			cout << "iSocketType: " << info.iSocketType << endl;
			cout << "iProtocol: " << info.iProtocol << endl;
		}
	}
	else {
		cout << "Failed to get socket options" << endl;
	}

	bool result = ConnectExOriginal(s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);
	cout << "Result " << result << endl;
	cout << "Error " << WSAGetLastError() << endl;
	return result;

}

int WINAPI sendtoReplaced(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen) {

	cout << "sendto detected" << endl;
	string targetIp = inet_ntoa(((sockaddr_in*)to)->sin_addr);
	string targetPort = std::to_string(htons(((sockaddr_in*)to)->sin_port));
	cout << "Target is " << targetIp << ":" << targetPort << endl;
	return sendtoOriginal(s, buf, len, flags, to, tolen);
}

int WINAPI recvfromReplaced(SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromlen) {
	cout << "recvfrom detected" << endl;
	string targetIp = inet_ntoa(((sockaddr_in*)from)->sin_addr);
	string targetPort = std::to_string(htons(((sockaddr_in*)from)->sin_port));
	cout << "Target is " << targetIp << ":" << targetPort << endl;
	return recvfromOriginal(s, buf, len, flags, from, fromlen);

}

int WINAPI WSASendToReplaced(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const sockaddr* lpTo, int iTolen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	WSAPROTOCOL_INFOW info;
	int size = sizeof(info);
	if (!(getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&info, &size) == 0 &&
		(info.iAddressFamily == AF_INET /*|| info.iAddressFamily == AF_INET6*/) &&
		info.iSocketType == SOCK_DGRAM && info.iProtocol == IPPROTO_UDP)) {

		string targetIp = inet_ntoa(((sockaddr_in*)lpTo)->sin_addr);
		string targetPort = std::to_string(htons(((sockaddr_in*)lpTo)->sin_port));
		cout << "WSASendTo detected " << s << " but it's not udp " << targetIp << ":" << targetPort << endl;
		return WSASendToOriginal(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine);
	}
	string targetIp = inet_ntoa(((sockaddr_in*)lpTo)->sin_addr);
	string targetPort = std::to_string(htons(((sockaddr_in*)lpTo)->sin_port));
	cout << "WSASendTo detected " << s << " " << targetIp << ":" << targetPort << endl;
	return WSASendToOriginal(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine);
}


int WINAPI WSARecvFromReplaced(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, sockaddr* lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	WSAPROTOCOL_INFOW info;
	int size = sizeof(info);
	if (!(getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&info, &size) == 0 &&
		(info.iAddressFamily == AF_INET /*|| info.iAddressFamily == AF_INET6*/) &&
		info.iSocketType == SOCK_DGRAM && info.iProtocol == IPPROTO_UDP)) {

		string targetIp = inet_ntoa(((sockaddr_in*)lpFrom)->sin_addr);
		string targetPort = std::to_string(htons(((sockaddr_in*)lpFrom)->sin_port));
		cout << "WSARecvFrom detected " << s << " but it's not udp " << targetIp << ":" << targetPort << endl;
		return WSARecvFromOriginal(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);
	}

	string targetIp = inet_ntoa(((sockaddr_in*)lpFrom)->sin_addr);
	string targetPort = std::to_string(htons(((sockaddr_in*)lpFrom)->sin_port));
	cout << "WSARecvFrom detected " << s << " " << targetIp << ":" << targetPort << endl;
	return WSARecvFromOriginal(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);
}

int WSAAPI sendReplaced(SOCKET s, const char* buf, int len, int flags) {
	cout << "send detected " << s << ", " << len << "bytes" << endl;
	cout << setfill('0');
	for (int i = 0; i < len/20 + 1; i++) {
		for (int j = 0; j < 20 && i * 20 + j < len; j++) {
			cout << hex << setw(2) << int(unsigned char((buf[i * 20 + j]))) << " ";
		}
		cout << endl;
	}
	cout << dec;
	return sendOriginal(s, buf, len, flags);
}

int WSAAPI recvReplaced(SOCKET s, char* buf, int len, int flags) {
	cout << "recv detected " << s << "... buff size: " << len <<  endl;
	int status = recvOriginal(s, buf, len, flags);
	cout << "recv'ed " << status << " bytes" << endl;
	cout << setfill('0');
	for (int i = 0; i < status / 20 + 1; i++) {
		for (int j = 0; j < 20 && i * 20 + j < status; j++) {
			cout << hex << setw(2) << int(unsigned char((buf[i * 20 + j]))) << " ";
		}
		cout << endl;
	}
	cout << dec;
	return status;
}

/*BOOL IsDebuggerPresentReplaced() {
	cout << "IsDebuggerPresent called and returned " << IsDebuggerPresentOriginal() << endl;
	return FALSE;
}*/
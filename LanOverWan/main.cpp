#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include "functions.h"
#include <dns_sd.h>
#include <thread>
#include "callbackTask.h"
#include "mainServer.h"
#include "mainClient.h"
#include <iphlpapi.h>
#include "configLoader.h"
using namespace std;


bool initiateFunctions();
void cleanupFunctions();

bool getLocalAddresses();

bool isServer = false;
bool functionsSet = false;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	//
	//int(fakeistream::*myfunc)() = &(fakeistream::getReplacemnt);
	if (DetourIsHelperProcess()) {
		return TRUE;
	}
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		cout << "DLL_PROCESS_ATTACH" << endl;
		cout << hModule << endl;
		string address, port;
		CFGL cfgl("WOLconfig.cfg", "//");
		if (cfgl == false || cfgl.exists("server") == -1 || cfgl.exists("port") == -1 || cfgl.exists("host") == -1) {

			cout << "Failed to load config" << endl;
			ofstream out("WOLconfig.cfg");
			out << "server=false //true or false" << endl;
			out << "port=" << DEFAULT_PORT << endl;
			out << "host=0.0.0.0" << endl;
			return false;
		}
		else {
			string temp;
			cfgl.request("server", "false", temp);
			isServer = (temp == "true");
			cfgl.request("port", DEFAULT_PORT, port);
			cfgl.request("host", "0.0.0.0", address);

		}
		cout << "server: " << isServer << endl;
		cout << "host: " << address << endl;
		cout << "port: " << port << endl;
		fServer = isServer;

		if (!isServer && !getLocalAddresses()) {
			cout << "Failed to get local addresses" << endl;
		}
		if (!initiateFunctions()) {
			cout << "Failed to detour" << endl;
			return false;
		}
		if (isServer) {

			thread initThread = thread(initServer, port);
			initThread.detach();
		}
		else {
			targetAddress = address;
			thread initThread = thread(initClient, address, port);
			initThread.detach();

		}
	}

	break;
	case DLL_THREAD_ATTACH:
		//out << 2 << endl;
		break;
	case DLL_THREAD_DETACH:
		//out << 3 << " " << asdf << endl;
		break;
	case DLL_PROCESS_DETACH:
		cout << "DLL_PROCESS_DETACH" << endl;
		if (functionsSet) {
			cleanupFunctions();
		}
		if (isServer) {
			cleanupServer();
		}
		else {
			cleanupClient();
		}
		break;
	}
	return TRUE;
}

bool initiateFunctions() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	connectOriginal = connect;
	WSAConnectOriginal = WSAConnect;
	WSAIoctlOriginal = WSAIoctl;
	sendtoOriginal = sendto;
	recvfromOriginal = recvfrom;
	WSASendToOriginal = WSASendTo;
	WSARecvFromOriginal = WSARecvFrom;
	sendOriginal = send;
	recvOriginal = recv;
	//WSAGetOverlappedResultOriginal = WSAGetOverlappedResult;



	WSAConnectByNameWOriginal = WSAConnectByNameW;
	WSAConnectByNameAOriginal = WSAConnectByNameA;
	WSAConnectByListOriginal = WSAConnectByList;
	IsDebuggerPresentOriginal = IsDebuggerPresent;
	DetourAttach((PVOID*)(&connectOriginal), connectReplaced);
	DetourAttach((PVOID*)(&WSAConnectOriginal), WSAConnectReplaced);
	//DetourAttach((PVOID*)(&WSAIoctlOriginal), WSAIoctlReplaced);
	//DetourAttach((PVOID*)(&sendtoOriginal), sendtoReplaced);
	//DetourAttach((PVOID*)(&recvfromOriginal), recvfromReplaced);
	//DetourAttach((PVOID*)(&WSASendToOriginal), WSASendToReplaced);
	//DetourAttach((PVOID*)(&WSARecvFromOriginal), WSARecvFromReplaced);
	//DetourAttach((PVOID*)(&sendOriginal), sendReplaced);
	//DetourAttach((PVOID*)(&recvOriginal), recvReplaced);
	//DetourAttach((PVOID*)(&IsDebuggerPresentOriginal), IsDebuggerPresentReplaced);

	//DetourAttach((PVOID*)(&WSAConnectByNameWOriginal), WSAConnectByNameWReplaced);
	//DetourAttach((PVOID*)(&WSAConnectByNameAOriginal), WSAConnectByNameAReplaced);
	//DetourAttach((PVOID*)(&WSAConnectByListOriginal), WSAConnectByListReplaced);
	/*if (ConnectExOriginal != NULL) {
		DetourAttach((PVOID*)(&ConnectExOriginal), ConnectExReplaced);

	}*/
	//DetourAttach((PVOID *)(&DispatchMessageOriginal), DispatchMessageReplaced);

	DetourTransactionCommit();
	//new std::thread(ts3plugin_configure, (void*)NULL, (void*)NULL);
	functionsSet = true;
	return true;
}

void cleanupFunctions() {

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach((PVOID*)(&connectOriginal), connectReplaced);
	DetourDetach((PVOID*)(&WSAConnectOriginal), WSAConnectReplaced);
	DetourTransactionCommit();
}

bool getLocalAddresses() {
	ULONG outBufLen = 15 * 1024;
	PMIB_IPADDRTABLE  pAddrTable = NULL;


	int iterations = 0;
	ULONG result;
	do {

		pAddrTable = (PMIB_IPADDRTABLE)new char[outBufLen];
		if (pAddrTable == NULL) {
			cout << "Failed to allocate table" << endl;
			return false;
		}

		result = GetIpAddrTable(pAddrTable, &outBufLen, true);

		if (result == ERROR_INSUFFICIENT_BUFFER) {
			delete[] pAddrTable;
			pAddrTable = NULL;
		}
		else {
			break;
		}

		iterations++;

	} while ((result == ERROR_INSUFFICIENT_BUFFER) && (iterations < 10));
	if (pAddrTable == NULL) {
		return false;
	}
	for (int i = 0; i < pAddrTable->dwNumEntries; i++) {
		struct in_addr addr;
		addr.s_addr = pAddrTable->table[i].dwAddr;
		string address = inet_ntoa(addr);
		if (address != "127.0.0.1") {
			myAddresses.insert(address);
		}
		cout << "Address of interface " << pAddrTable->table[i].dwIndex << ": " << inet_ntoa(addr) << endl;
	}
	return true;
}
	


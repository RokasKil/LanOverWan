#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include "functions.h"
#include <dns_sd.h>
#include <thread>
#include "WOLServer.h"
#include "callbackTask.h"
#include "mainServer.h"

using namespace std;


bool initiateFunctions();
static void DNSSD_API reg_reply(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode,
	const char* name, const char* regtype, const char* domain, void* context);

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
		//initiateFunctions();
		if (server) {

			thread initThread = thread(initServer, (string)DEFAULT_PORT);
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
		if (server) {
			cleanupServer();
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
	DetourAttach((PVOID*)(&WSAIoctlOriginal), WSAIoctlReplaced);
	DetourAttach((PVOID*)(&sendtoOriginal), sendtoReplaced);
	DetourAttach((PVOID*)(&recvfromOriginal), recvfromReplaced);
	DetourAttach((PVOID*)(&WSASendToOriginal), WSASendToReplaced);
	DetourAttach((PVOID*)(&WSARecvFromOriginal), WSARecvFromReplaced);
	DetourAttach((PVOID*)(&sendOriginal), sendReplaced);
	DetourAttach((PVOID*)(&recvOriginal), recvReplaced);
	DetourAttach((PVOID*)(&IsDebuggerPresentOriginal), IsDebuggerPresentReplaced);

	DetourAttach((PVOID*)(&WSAConnectByNameWOriginal), WSAConnectByNameWReplaced);
	DetourAttach((PVOID*)(&WSAConnectByNameAOriginal), WSAConnectByNameAReplaced);
	DetourAttach((PVOID*)(&WSAConnectByListOriginal), WSAConnectByListReplaced);
	if (ConnectExOriginal != NULL) {
		DetourAttach((PVOID*)(&ConnectExOriginal), ConnectExReplaced);

	}
	//DetourAttach((PVOID *)(&DispatchMessageOriginal), DispatchMessageReplaced);

	DetourTransactionCommit();
	//new std::thread(ts3plugin_configure, (void*)NULL, (void*)NULL);
	return true;
}



static void DNSSD_API reg_reply(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode,
	const char* name, const char* regtype, const char* domain, void* context) {
	cout << "reg reply for " << name << " " << regtype << " at " << domain << endl;
	/*DNSRecordRef recorder;
	DNSServiceErrorType err = DNSServiceAddRecord(sdref, &recorder, 0, service.data->rrtype, service.data->rdlen, service.data->rdata, 0);
	if (err == kDNSServiceErr_NoError) {
		cout << "Succesfully added record" << endl;
	}
	else {
		cout << "Failed to add record" << endl;
	}*/
}
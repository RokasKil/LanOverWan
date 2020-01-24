#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include "functions.h"
#include <dns_sd.h>
#include <thread>
using namespace std;

bool initiateFunctions();
DNSServiceRef client;
static void DNSSD_API browse_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* replyName, const char* replyType, const char* replyDomain, void* context);

void browseTask();
thread browseThread;
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
		cout << DNSServiceGetProperty << endl;
		//initiateFunctions();
		browseThread = thread (browseTask);
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

static void DNSSD_API browse_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* replyName, const char* replyType, const char* replyDomain, void* context) {
	if (flags & kDNSServiceFlagsAdd) {

		cout << "Added game ";
	}
	else {
		cout << "Removed game ";
	}
	cout << "(" << ifIndex << "): " << replyName << endl;
}

void browseTask()
{
	DNSServiceErrorType err = DNSServiceBrowse(&client, 0, kDNSServiceInterfaceIndexAny, "_blizzard._udp,_w3xp272f", "", browse_reply, NULL);
	if (err == kDNSServiceErr_NoError) {
		cout << "Service browser started successfully" << endl;
	}
	else {
		cout << "Service browser failed " << err << endl;
		return;
	}
	while (DNSServiceProcessResult(client) == kDNSServiceErr_NoError) {
		cout << "Result recieved" << endl;
	}
	DNSServiceRefDeallocate(client);

}
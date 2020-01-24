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
static void DNSSD_API resolve_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context);
static void DNSSD_API qr_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context);

void browseTask();
thread browseThread;

void handleCallbacksTask(DNSServiceRef service, string name);

void handleCallbacksTask(DNSServiceRef service, string name, bool deallocate);

#define handleCallbacksTaskPointer static_cast<void (*)(DNSServiceRef, string)>(handleCallbacksTask)

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
	bool added = flags & kDNSServiceFlagsAdd;
	if (added) {

		cout << "Added game ";
	}
	else {
		cout << "Removed game ";
	}
	cout << "(" << ifIndex << "): " << replyName << endl;
	if (added) {
		DNSServiceRef resolver;
		DNSServiceErrorType err = DNSServiceResolve(&resolver, 0, ifIndex, replyName, replyType, replyDomain, resolve_reply, NULL);
		if (err == kDNSServiceErr_NoError) {
			thread resolveThread(handleCallbacksTaskPointer, resolver, "Resolver");
			resolveThread.detach();
		}
		else {
			cout << "Failed to resolve service " << err << endl;
		}
	}
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
	handleCallbacksTask(client, "Browser");

}


void handleCallbacksTask(DNSServiceRef service, string name) {
	handleCallbacksTask(service, name, true);
}

void handleCallbacksTask(DNSServiceRef service, string name, bool deallocate) {
	DNSServiceErrorType err;
	while ((err = DNSServiceProcessResult(service)) == kDNSServiceErr_NoError);
	if (name != "") {
		cout << name << " stopped :" << err << endl;
	}
	if (deallocate) {
		DNSServiceRefDeallocate(service);
	}
}

static void DNSSD_API resolve_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context) {
	cout << "Resolved " << fullname << " on (" << ifIndex << "): " << hosttarget << ":" << htons(port) << " flags: " << flags << " txtLen: " << txtLen << endl;
	DNSServiceRef query;
	// No idea what type is 0x42 (66) but it's what warcraft uses it and it gives me my data :)
	DNSServiceErrorType err = DNSServiceQueryRecord(&query, kDNSServiceFlagsLongLivedQuery, ifIndex, fullname, 0x42, kDNSServiceClass_IN, qr_reply, NULL);
	if (err == kDNSServiceErr_NoError) {
		thread queryThread(handleCallbacksTaskPointer, query, "Query");
		queryThread.detach();
	}
	else {
		cout << "Failed to query TXT " << err << endl;
	}
	//only possible flag should be 0x1
}


static void DNSSD_API qr_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context) {
	cout << "Queried " << fullname << " on (" << ifIndex << ") flags: " << flags << " rrtype: " << rrtype << " rdlen: " << rdlen << " ttl: " << ttl << endl;
	int count = TXTRecordGetCount(rdlen, rdata);
	cout << "Found " << count << " entries" << endl;
	char key[256];
	const void* value;
	uint8_t valueLen;
	for (int i = 0; i < count; i++) {
		TXTRecordGetItemAtIndex(rdlen, rdata, i, 256, key, &valueLen, &value);
		cout << key << ": " << string((char*)value, (char*)value + valueLen) << endl;
	}
}
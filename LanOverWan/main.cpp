#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include "functions.h"
#include <dns_sd.h>
#include <thread>
#include "WOLServer.h"
using namespace std;

const char BLIZZARD_TYPE[] = "_blizzard._udp,_w3xp272f";

bool initiateFunctions();
DNSServiceRef client;
static void DNSSD_API browse_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* replyName, const char* replyType, const char* replyDomain, void* context);
static void DNSSD_API resolve_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context);
static void DNSSD_API qr_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context);
static void DNSSD_API reg_reply(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode,
	const char* name, const char* regtype, const char* domain, void* context);

void browseTask();
thread browseThread;

void handleCallbacksTask(DNSServiceRef service, string name);

void handleCallbacksTask(DNSServiceRef service, string name, bool deallocate);

int callbackHandlers = 0;

struct ResolveServiceRef {
	DNSServiceRef *resolve = NULL;
	DNSServiceRef *query = NULL;
};

map<string, pair<WarcraftSerivce, uint32_t>> services;
map<string, string> fullToName;
map<string, bool> nameRecorded;
map<pair<string, uint32_t>, ResolveServiceRef> resolves;
bool isFullNameRecorded(string fullName) {
	auto entry = fullToName.find(fullName);
	return entry != fullToName.end();

}

bool isNameRecorded(string name) {
	auto entry = nameRecorded.find(name);
	return entry != nameRecorded.end() && (*entry).second;
}

void recordName(string fullName, string name) {
	fullToName[fullName] = name;
	nameRecorded[name] = true;
}
WOLServer* server = NULL;
void init();
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
		//initiateFunctions();
		thread initThread = thread(init);
		initThread.detach();
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
		DNSServiceRefDeallocate(client);
		cout << "DLL_PROCESS_DETACH1" << endl;
		if (server != NULL) {
			delete server;
		}
		cout << "DLL_PROCESS_DETACH2" << endl;
		for (auto i = resolves.begin(); i != resolves.end(); i++) {
			if ((*i).second.query != NULL) {
				DNSServiceRefDeallocate(*(*i).second.query);
				delete (*i).second.query;
			}
			if ((*i).second.resolve != NULL) {
				DNSServiceRefDeallocate(*(*i).second.resolve);
				delete (*i).second.resolve;
			}
		}
		cout << "DLL_PROCESS_DETACH3" << endl;
		for (auto i = services.begin(); i != services.end(); i++) {
			if ((*i).second.first.data != NULL) {
				if ((*i).second.first.data->rdata != NULL) {
					delete[](*i).second.first.data->rdata;
				}
				delete (*i).second.first.data;
			}
		}
		cout << "DLL_PROCESS_DETACH4" << endl;
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
		auto entry = services.find(replyName);
		if (entry == services.end()) {
			WarcraftSerivce service;
			service.name = replyName;
			service.type = BLIZZARD_TYPE;
			services[replyName] = { service, ifIndex };
		}
		string* strP = NULL;
		if (!isNameRecorded(replyName)) { // Might be a very small memory leak and for this project I'm willing to let it slide
			strP = new string(replyName);
		}
		ResolveServiceRef refs;
		refs.resolve = new DNSServiceRef();
		DNSServiceErrorType err = DNSServiceResolve(refs.resolve, 0, ifIndex, replyName, replyType, replyDomain, resolve_reply, (void*)strP);

		if (err == kDNSServiceErr_NoError) {
			resolves[{replyName, ifIndex}] = refs;
			thread resolveThread(handleCallbacksTaskPointer, *refs.resolve, "Resolver");
			resolveThread.detach();
		}
		else {
			if(strP != NULL)
				delete strP;
			delete refs.resolve;

			cout << "Failed to resolve service " << err << endl;
		}
	}
	else {
		auto entry = services.find(replyName);
		if (entry != services.end() && (*entry).second.second == ifIndex) {
			server->removeService((*entry).second.first);
			if ((*entry).second.first.data != NULL) {
				if ((*entry).second.first.data->rdata != NULL) {
					delete[] (*entry).second.first.data->rdata;
				}
				delete (*entry).second.first.data;
			}
			services.erase(entry);
		}

		auto entryR = resolves.find({ replyName, ifIndex });
		if (entryR != resolves.end()) {
			if ((*entryR).second.query != NULL) {
				DNSServiceRefDeallocate(*(*entryR).second.query);
				delete (*entryR).second.query;
			}
			if ((*entryR).second.resolve != NULL) {
				DNSServiceRefDeallocate(*(*entryR).second.resolve);
				delete (*entryR).second.resolve;
			}
			resolves.erase(entryR);
		}
		/*if (serviceStage == 3) {

			DNSServiceRef registerer;
			DNSServiceErrorType err = DNSServiceRegister(&registerer, kDNSServiceFlagsNoAutoRename, kDNSServiceInterfaceIndexAny, (service.name + "WAN").c_str(), service.type.c_str(), NULL, NULL, htons(service.port), 1, NULL, reg_reply, NULL);
			if (err == kDNSServiceErr_NoError) {
				thread registererThread(handleCallbacksTaskPointer, registerer, "Registerer");
				registererThread.detach();
			}
			serviceStage++;
		}*/
	}
}
void init() {

	server = new WOLServer();
	if (!server->startListening() || !server->valid) {
		cout << "Failed to start server " << endl;
		return;
	}
	browseThread = thread(browseTask);
	browseThread.detach();
}

void browseTask()
{
	DNSServiceErrorType err = DNSServiceBrowse(&client, 0, kDNSServiceInterfaceIndexAny, BLIZZARD_TYPE, "", browse_reply, NULL);
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
	callbackHandlers++;
	while ((err = DNSServiceProcessResult(service)) == kDNSServiceErr_NoError);
	if (name != "") {
		cout << name << " stopped :" << err << endl;
	}
	if (deallocate) {
		DNSServiceRefDeallocate(service);
	}
	callbackHandlers--;
}

static void DNSSD_API resolve_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context) {
	cout << "Resolved " << fullname << " on (" << ifIndex << "): " << hosttarget << ":" << ntohs(port) << " flags: " << flags << " txtLen: " << txtLen << endl;
	/*if (serviceStage == 1) {
		service.port = ntohs(port);
		serviceStage++;
	}*/
	if (!isFullNameRecorded(fullname)) {
		string* replyName = (string*)context;
		fullToName[fullname] = *replyName;
		delete replyName;
		replyName = NULL;
	}
	string name = fullToName[fullname];
	auto entry = services.find(name);
	if (entry != services.end() && (*entry).second.second == ifIndex) {
		(*entry).second.first.port = ntohs(port);
	}
	DNSServiceRef query;
	// No idea what type is 0x42 (66) but it's what warcraft uses it and it gives me my data :)
	resolves[{name, ifIndex}].query = new DNSServiceRef();
	DNSServiceErrorType err = DNSServiceQueryRecord(resolves[{name, ifIndex}].query, kDNSServiceFlagsLongLivedQuery, ifIndex, fullname, 0x42, kDNSServiceClass_IN, qr_reply, NULL);
	if (err == kDNSServiceErr_NoError) {
		thread queryThread(handleCallbacksTaskPointer, *resolves[{name, ifIndex}].query, "Query");
		queryThread.detach();
	}
	else {
		cout << "Failed to query data " << err << endl;
		delete resolves[{name, ifIndex}].query;
		resolves[{name, ifIndex}].query = NULL;

	}
	//only possible flag should be 0x1
}


static void DNSSD_API qr_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context) {
	cout << "Queried " << fullname << " on (" << ifIndex << ") flags: " << flags << " rrtype: " << rrtype << " rdlen: " << rdlen << " ttl: " << ttl << endl;
	/*if (serviceStage == 2) {
		service.data = new warcraftServiceData();
		service.data->rrtype = rrtype;
		service.data->rrclass = rrclass;
		service.data->rdlen = rdlen;
		service.data->rdata = new char[rdlen];
		memcpy(service.data->rdata, rdata, rdlen);
		serviceStage++;
	}*/
	if (flags & kDNSServiceFlagsAdd) {
		string name = fullToName[fullname];
		auto entry = services.find(name);
		if (entry != services.end() && (*entry).second.second == ifIndex) {
			(*entry).second.first.data = new WarcraftServiceData();
			(*entry).second.first.data->rrtype = rrtype;
			(*entry).second.first.data->rdlen = rdlen;
			(*entry).second.first.data->rdata = new char[rdlen];
			memcpy((*entry).second.first.data->rdata, rdata, rdlen);
			server->addService((*entry).second.first);
		}
	}
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
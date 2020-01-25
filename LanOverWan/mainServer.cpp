#include "mainServer.h"

map<string, pair<WarcraftService, uint32_t>> services;
map<string, string> fullToName;
map<string, bool> nameRecorded;
map<pair<string, uint32_t>, ResolveServiceRef> resolves;

DNSServiceRef client;

WOLServer* server = NULL;


void initServer(string port) {
	server = new WOLServer(port);
	if (!server->startListening() || !server->valid) {
		cout << "Failed to start server " << endl;
		MessageBox(NULL, L"Failed to start server", NULL, NULL);
		return;
	}
	thread browseThread = thread(browseTask);
	browseThread.detach();
}

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
			WarcraftService service;
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
			if (strP != NULL)
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
					delete[](*entry).second.first.data->rdata;
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



			serviceStage++;
		}*/
	}
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

void cleanupServer() {
	DNSServiceRefDeallocate(client);
	if (server != NULL) {
		delete server;
	}
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
	for (auto i = services.begin(); i != services.end(); i++) {
		if ((*i).second.first.data != NULL) {
			if ((*i).second.first.data->rdata != NULL) {
				delete[](*i).second.first.data->rdata;
			}
			delete (*i).second.first.data;
		}
	}
}

void browseTask()
{
	DNSServiceErrorType err = DNSServiceBrowse(&client, 0, kDNSServiceInterfaceIndexAny, BLIZZARD_TYPE, "", browse_reply, NULL);
	if (err == kDNSServiceErr_NoError) {
		cout << "Service browser started successfully" << endl;
	}
	else {
		cout << "Service browser failed " << err << endl;
		MessageBox(NULL, L"Failed to start browser", NULL, NULL);
		return;
	}
	handleCallbacksTask(client, "Browser");

}
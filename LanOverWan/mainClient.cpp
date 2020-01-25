#include "mainClient.h"
WOLClient* client;


map<string, pair<WarcraftService, DNSServiceRef>> services;
map<string, DNSRecordRef> records;

void initClient(string address, string port) {
	client = new WOLClient(address, port);
	client->setAddCallback((AddServiceCallback)addServiceCallback);
	client->setUpdateCallback((UpdateServiceCallback)updateServiceCallback);
	client->setRemoveCallback((RemoveServiceCallback)removeServiceCallback);
	client->setDisconnectCallback((DisconnectedCallback)disconnectedCallback);
	if (client->valid) {
		client->connectToServer();
	}
	
}


void cleanupClient() {
	client->setDisconnectCallback(NULL);
	client->disconnect();
	delete client;
}

void WOL_API addServiceCallback(WOLClient* client, WarcraftService service) {
	cout << "Adding service " << service.name << " " << service.type << " " << service.port << endl;
	auto entry = services.find(service.name);
	if (entry != services.end()) {
		deleteServiceData((*entry).second.first.data);
		DNSServiceRefDeallocate((*entry).second.second);
	}
	DNSServiceRef registerer;
	DNSServiceErrorType err = DNSServiceRegister(&registerer, kDNSServiceFlagsNoAutoRename, kDNSServiceInterfaceIndexAny, (service.name).c_str(), service.type.c_str(), NULL, NULL, htons(service.port), 1, NULL, reg_reply, NULL);
	if (err == kDNSServiceErr_NoError) {
		services[service.name] = { service, registerer };
		thread registererThread(handleCallbacksTaskPointer, registerer, "Registerer");
		registererThread.detach();
	}
	else {
		cout << "Failed to register " << service.name << endl;
	}
	
}

void WOL_API updateServiceCallback(WOLClient* client, string name, WarcraftServiceData* data) {
	cout << "Updating service " << name << " " << data->rdlen << endl;
	auto entry = services.find(name);
	if (entry != services.end()) {
		uint16_t rrtype = (*entry).second.first.data->rrtype;
		deleteServiceData((*entry).second.first.data);
		(*entry).second.first.data = data;
		(*entry).second.first.data->rrtype = rrtype;

		auto entryR = records.find(name);
		if (entryR != records.end()) {
			DNSServiceErrorType err = DNSServiceUpdateRecord((*entry).second.second, (*entryR).second, 0, (*entry).second.first.data->rdlen, (*entry).second.first.data->rdata, 0);
			if (err == kDNSServiceErr_NoError) {
				cout << "Succesfully updated record" << endl;
			}
			else {
				cout << "Failed to update record" << endl;
			}
		}
		else {
			cout << "No record reference found" << endl;
		}
	}
	else {
		cout << "No service found" << endl;
		deleteServiceData(data);
	}
}

void WOL_API removeServiceCallback(WOLClient* client, string name) {
	cout << "Remove service: " << name << endl;;
	auto entry = services.find(name);
	if (entry != services.end()) {
		deleteServiceData((*entry).second.first.data);
		DNSServiceRefDeallocate((*entry).second.second);
		services.erase(entry);
	}
	auto entryR = records.find(name);
	if (entryR != records.end()) {
		records.erase(entryR);
	}
}

void WOL_API disconnectedCallback(WOLClient* client) {
	cout << "Disconnected" << endl;
	for (auto i = services.begin(); i != services.end(); i++) {
		deleteServiceData((*i).second.first.data);
		DNSServiceRefDeallocate((*i).second.second);
	}
	services.clear();
	records.clear();
}

static void DNSSD_API reg_reply(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode,
	const char* name, const char* regtype, const char* domain, void* context) {
	cout << "reg reply for " << name << " " << regtype << " at " << domain << endl;

	auto entry = services.find(name);
	if (entry != services.end()) {
		DNSRecordRef recorder;
		DNSServiceErrorType err = DNSServiceAddRecord(sdref, &recorder, 0, (*entry).second.first.data->rrtype, (*entry).second.first.data->rdlen, (*entry).second.first.data->rdata, 0);
		if (err == kDNSServiceErr_NoError) {
			cout << "Succesfully added record" << endl;
			records[name] = recorder;
		}
		else {
			cout << "Failed to add record" << endl;
		}
	}
	else {
		cout << "Didn't find tied service" << endl;
	}

}
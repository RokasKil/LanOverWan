#pragma once

#include <iostream>
#include <map>
#include "functions.h"
#include <dns_sd.h>
#include <thread>
#include "WOLClient.h"
#include "callbackTask.h"
#include "WOLConstants.h"

using namespace std;

void WOL_API addServiceCallback (WOLClient* client, WarcraftService service);
void WOL_API updateServiceCallback (WOLClient* client, string name, WarcraftServiceData* data);
void WOL_API removeServiceCallback (WOLClient* client, string name);
void WOL_API disconnectedCallback (WOLClient* client);

static void DNSSD_API reg_reply(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode,
	const char* name, const char* regtype, const char* domain, void* context);

void initClient(string address, string port);
void cleanupClient();
#pragma once

#include <iostream>
#include <map>
#include "functions.h"
#include <dns_sd.h>
#include <thread>
#include "WOLServer.h"
#include "callbackTask.h"
#include "WOLConstants.h"

using namespace std;


struct ResolveServiceRef {
	DNSServiceRef* resolve = NULL;
	DNSServiceRef* query = NULL;
};



bool isFullNameRecorded(string fullName);

bool isNameRecorded(string name);

void recordName(string fullName, string name);

void initServer(string port);

void cleanupServer();

void browseTask();

static void DNSSD_API browse_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* replyName, const char* replyType, const char* replyDomain, void* context);
static void DNSSD_API resolve_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context);
static void DNSSD_API qr_reply(DNSServiceRef sdref, const DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType errorCode,
	const char* fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context);
#pragma once

#include <string>

struct WarcraftServiceData {
	uint16_t rrtype, rdlen;
	void* rdata = NULL;
};

struct WarcraftService {
	std::string name, type;
	uint16_t port;
	WarcraftServiceData* data = NULL;
};

void deleteServiceData(WarcraftServiceData* data);
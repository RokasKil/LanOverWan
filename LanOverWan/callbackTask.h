#pragma once

#include <dns_sd.h>
#include <string>
#include <iostream>

#define handleCallbacksTaskPointer static_cast<void (*)(DNSServiceRef, string)>(handleCallbacksTask)

void handleCallbacksTask(DNSServiceRef service, std::string name);

void handleCallbacksTask(DNSServiceRef service, std::string name, bool deallocate);
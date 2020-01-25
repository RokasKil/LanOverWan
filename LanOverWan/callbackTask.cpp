#include "callbackTask.h"

void handleCallbacksTask(DNSServiceRef service, std::string name) {
	handleCallbacksTask(service, name, true);
}

void handleCallbacksTask(DNSServiceRef service, std::string name, bool deallocate) {
	DNSServiceErrorType err;
	while ((err = DNSServiceProcessResult(service)) == kDNSServiceErr_NoError);
	if (name != "") {
		std::cout << name << " stopped :" << err << std::endl;
	}
	if (deallocate) {
		DNSServiceRefDeallocate(service);
	}
}
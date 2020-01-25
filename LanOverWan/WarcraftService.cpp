#include "WarcraftService.h"

void deleteServiceData(WarcraftServiceData* data) {
	if (data != NULL) {
		if (data->rdata != NULL) {
			delete[] data->rdata;
		}
		delete data;
	}
}

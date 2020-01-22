
#include <windows.h>
#include <iostream>
#include <detours.h>
#include "functions.h"
using namespace std;

bool initiateFunctions();

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
		cout << "DLL_PROCESS_ATTACH" << endl;
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

	sendtoOriginal = sendto;
	recvfromOriginal = recvfrom;
	WSASendToOriginal = WSASendTo;
	WSARecvFromOriginal = WSARecvFrom;
	//WSAGetOverlappedResultOriginal = WSAGetOverlappedResult;



	WSAConnectByNameWOriginal = WSAConnectByNameW;
	WSAConnectByNameAOriginal = WSAConnectByNameA;
	WSAConnectByListOriginal = WSAConnectByList;

	DetourAttach((PVOID*)(&connectOriginal), connectReplaced);
	DetourAttach((PVOID*)(&WSAConnectOriginal), WSAConnectReplaced);



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
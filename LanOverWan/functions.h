#pragma once
#define WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <iostream>
#include <detours.h>

typedef int(WSAAPI* connectFunction)(SOCKET socket, const sockaddr* name, int namelen);
typedef int(WSAAPI* WSAConnectFunction)(SOCKET socket, const sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS);
typedef int(WSAAPI* ioctlsocketFunction)(SOCKET s, long cmd, u_long* argp);
typedef int(WSAAPI* WSAIoctlFunction)(SOCKET s, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
typedef int(WSAAPI* WSAAsyncSelectFunction)(SOCKET s, HWND hWnd, u_int wMsg, long lEvent);
typedef int(WSAAPI* WSAEventSelectFunction)(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents);
typedef int(WSAAPI* sendtoFunction)(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen);
typedef int(WSAAPI* recvfromFunction)(SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromlen);
typedef int(WSAAPI* WSASendToFunction)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const sockaddr* lpTo, int iTolen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
typedef int(WSAAPI* WSARecvFromFunction)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, sockaddr* lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
typedef int(WSAAPI* closesocketFunction)(SOCKET s);
typedef BOOL(WSAAPI* WSAGetOverlappedResultFunction)(SOCKET s, LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer, BOOL fWait, LPDWORD lpdwFlags);
typedef DWORD(WSAAPI* WSAWaitForMultipleEventsFunction)(DWORD cEvents, const WSAEVENT* lphEvents, BOOL fWaitAll, DWORD dwTimeout, BOOL fAlertable);
typedef LRESULT(WSAAPI* DispatchMessageFunction)(const MSG* lpMsg);
typedef BOOL(WSAAPI* GetOverlappedResultFunction)(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait);
typedef BOOL(WSAAPI* GetOverlappedResultExFunction)(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, DWORD dwMilliseconds, BOOL bAlertable);
typedef BOOL(WSAAPI* GetQueuedCompletionStatusFunction)(
	HANDLE       CompletionPort,
	LPDWORD      lpNumberOfBytesTransferred,
	PULONG_PTR   lpCompletionKey,
	LPOVERLAPPED* lpOverlapped,
	DWORD        dwMilliseconds
	);
typedef BOOL(WSAAPI* GetQueuedCompletionStatusExFunction)(
	HANDLE             CompletionPort,
	LPOVERLAPPED_ENTRY lpCompletionPortEntries,
	ULONG              ulCount,
	PULONG             ulNumEntriesRemoved,
	DWORD              dwMilliseconds,
	BOOL               fAlertable
	);
typedef HANDLE(WSAAPI* CreateIoCompletionPortFunction)(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
typedef BOOL(WSAAPI* FreeLibraryFunction)(HMODULE hLibModule);
typedef BOOL(WSAAPI* CloseHandleFunction)(HANDLE hObject);

typedef BOOL(WSAAPI* WSAConnectByNameWFunction)(SOCKET s, LPWSTR nodename, LPWSTR servicename, LPDWORD LocalAddressLength, LPSOCKADDR LocalAddress, LPDWORD RemoteAddressLength, LPSOCKADDR RemoteAddress, const timeval* timeout, LPWSAOVERLAPPED Reserved);

typedef BOOL(WSAAPI* WSAConnectByNameAFunction)(SOCKET s, LPCSTR nodename, LPCSTR servicename, LPDWORD LocalAddressLength, LPSOCKADDR LocalAddress, LPDWORD RemoteAddressLength, LPSOCKADDR RemoteAddress, const timeval* timeout, LPWSAOVERLAPPED Reserved);

typedef BOOL(WSAAPI* WSAConnectByListFunction)(SOCKET s, PSOCKET_ADDRESS_LIST SocketAddress, LPDWORD LocalAddressLength, LPSOCKADDR LocalAddress, LPDWORD RemoteAddressLength, LPSOCKADDR RemoteAddress, const timeval* timeout, LPWSAOVERLAPPED Reserved);


extern connectFunction connectOriginal;
extern WSAConnectFunction WSAConnectOriginal;
extern ioctlsocketFunction ioctlsocketOriginal;
extern WSAIoctlFunction WSAIoctlOriginal;
extern WSAAsyncSelectFunction WSAAsyncSelectOriginal;
extern WSAEventSelectFunction WSAEventSelectOriginal;
extern sendtoFunction sendtoOriginal;
extern recvfromFunction recvfromOriginal;
extern WSASendToFunction WSASendToOriginal;
extern WSARecvFromFunction WSARecvFromOriginal;
extern closesocketFunction closesocketOriginal;
extern WSAGetOverlappedResultFunction WSAGetOverlappedResultOriginal;
extern WSAWaitForMultipleEventsFunction WSAWaitForMultipleEventsOriginal;
extern DispatchMessageFunction DispatchMessageOriginal;
extern GetOverlappedResultFunction GetOverlappedResultOriginal;
extern GetOverlappedResultExFunction GetOverlappedResultExOriginal;
extern GetQueuedCompletionStatusFunction GetQueuedCompletionStatusOriginal;
extern GetQueuedCompletionStatusExFunction GetQueuedCompletionStatusExOriginal;
extern CreateIoCompletionPortFunction CreateIoCompletionPortOriginal;
extern FreeLibraryFunction FreeLibraryOriginal;
extern CloseHandleFunction CloseHandleOriginal;
extern WSAConnectByNameWFunction WSAConnectByNameWOriginal;
extern WSAConnectByNameAFunction WSAConnectByNameAOriginal;
extern WSAConnectByListFunction WSAConnectByListOriginal;
extern LPFN_CONNECTEX ConnectExOriginal;
extern LPFN_CONNECTEX ConnectExReturned;

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "LifeAndDeathOfTCPConnection.h"
#include "SharedHeaders.h"

void InitWinSock() {
	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}
}

SOCKET Create_Socket() {
	if (socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) == INVALID_SOCKET ){
			printf("Error at socket( ): %ld\n", WSAGetLastError());
			return INVALID_SOCKET;
		}
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

SOCKADDR_IN GetService() {
	SOCKADDR_IN service;
	unsigned long Address;
	// Create a sockaddr_in object and set its values.
	// Declare variables

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(SERVER_PORT);
	return service;
}

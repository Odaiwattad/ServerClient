# ServerClient
school project
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
 This file was written for instruction purposes for the
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
 /*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "CallServer.h"
#include "SharedHeaders.h"
#include "SocketSendRecvTools.h"
#include "LifeAndDeathOfTCPConnection.h"
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread(SOCKET *t_socket);
static DWORD Check_exit_server();
static int Accept_Socket(SOCKET *t_socket);
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
typedef struct Parameters_to_serviceThread {
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
	char* Client_UserName;
	int number_of_players;
	}Client_parameters;

void CallServer()
{
	Client_parameters Client_parm;
	BOOL ret_val;
	DWORD exit_code, res;
	int number_of_clients_connected;
	SOCKET MainSocket = INVALID_SOCKET;
	SOCKET AcceptSocket = INVALID_SOCKET;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	HANDLE exit_handle;
	int Accepted;

	InitWinSock();

	/* The WinSock DLL is acceptable. Proceed. */

	// Create a socket.    
	MainSocket = Create_Socket();
	if (MainSocket == INVALID_SOCKET)
	{
		goto server_cleanup_1;
	}

	service = GetService();
	// Bind the socket.
	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
	}
	if (service.sin_addr.s_addr == INADDR_NONE || bindRes == SOCKET_ERROR) {
		goto server_cleanup_2;
	}


	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (number_of_clients_connected = 0; number_of_clients_connected < NUM_OF_WORKER_THREADS; number_of_clients_connected++)
		ThreadHandles[number_of_clients_connected] = NULL;

	printf("Waiting for a client to connect...\n");
	exit_handle = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Check_exit_server,
		NULL,
		0,
		NULL
	);

Check_if_exit:
	while(1)// needs change , it can be infinite loop
	{	

		ret_val = GetExitCodeThread(exit_handle, &exit_code);
		Sleep(10);
		if (exit_code == 1) { 
			res=WaitForSingleObject(exit_handle, 0);
			if (res == WAIT_OBJECT_0) {//debuged and working
				CloseHandle(exit_handle);
			}
			goto server_cleanup_3; }
		Accepted =Accept_Socket(MainSocket);
		if (Accepted == 1) {
			AcceptSocket = accept(MainSocket, NULL, NULL);
			if (AcceptSocket == INVALID_SOCKET)
			{
				printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
				goto server_cleanup_3;
			}
		}
		else{
			goto Check_if_exit;
		}
		printf("Client Connected.\n");

		number_of_clients_connected = FindFirstUnusedThreadSlot();

		if (number_of_clients_connected == NUM_OF_WORKER_THREADS) //no slot is available
		{
			printf("No slots available for client, dropping the connection.\n");
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		}
		else
		{
			Client_parm.number_of_players = number_of_clients_connected;
			Client_parm.ThreadInputs[number_of_clients_connected] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // ThreadInputs[Ind] when the
											  // time comes.
			ThreadHandles[number_of_clients_connected] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				&Client_parm,
				0,
				NULL
			);
		}
	} // for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )

server_cleanup_3:
	if (exit_code == 1) {
		TerminateThread(ThreadHandles[0], 1);
		TerminateThread(ThreadHandles[1], 1);
	}
	CleanupWorkerThreads();

server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static void CleanupWorkerThreads()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], INFINITE);

			if (Res == WAIT_OBJECT_0)
			{
				closesocket(ThreadInputs[Ind]);
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
				printf("Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
static DWORD ServiceThread(Client_parameters *Client_parm)
{
	char SendStr[SEND_STR_SIZE];

	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;



	while (!Done)
	{
		char *AcceptedStr = NULL;
		char Message_Type[MESSAGE_TYPE_MAX_LENGTH];
		
		RecvRes = ReceiveString(&AcceptedStr, (Client_parm->ThreadInputs[Client_parm->number_of_players]));
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			closesocket((Client_parm->ThreadInputs[Client_parm->number_of_players]));
			return 1;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			closesocket((Client_parm->ThreadInputs[Client_parm->number_of_players]));
			return 1;
		}
		else 
		{
			strcpy(Message_Type, Get_Message_Type(AcceptedStr));
			printf("Message Type : %s", Message_Type);
		}

		if (STRINGS_ARE_EQUAL(AcceptedStr, CLIENT_REQUEST))///Here we must check if 2 Clients are already in and Deny The third Cliend 
		{                                                 /// number of clients Connected is Client_parm->number_of_players
			if (Client_parm->number_of_players < 2) {
				strcpy(SendStr, SERVER_APPROVED);
				SendRes = SendString(SendStr, (Client_parm->ThreadInputs[Client_parm->number_of_players]));
				strcpy(SendStr, SERVER_MAIN_MENU);
			}
			else
			{
				strcpy(SendStr, SERVER_DENIED);
				strcat(SendStr, ":Maximum players are reached\n");
				
			}
		}
		else if (STRINGS_ARE_EQUAL(AcceptedStr, CLIENT_CPU ))
		{
			strcpy(SendStr, "great");
		}
		else if (STRINGS_ARE_EQUAL(AcceptedStr, "bye"))
		{
			strcpy(SendStr, "see ya!");
			Done = TRUE;
		}
		else
		{
			strcpy(SendStr, "I don't understand");
		}

		SendRes = SendString(SendStr, (Client_parm->ThreadInputs[Client_parm->number_of_players]));

		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			closesocket((Client_parm->ThreadInputs[Client_parm->number_of_players]));
			return 1;
		}

		free(AcceptedStr);
	}

	printf("Conversation ended.\n");
	closesocket((Client_parm->ThreadInputs[Client_parm->number_of_players]));
	return 0;
}

static DWORD Check_exit_server(void) {
	DWORD result;
	char Server_input[256];
	gets_s(Server_input, sizeof(Server_input));
	while (1) {
		if (STRINGS_ARE_EQUAL(Server_input, "exit")) { result= 1; 
		return result;
		}
	}
}
int Accept_Socket(SOCKET MainSocket) {
	fd_set set;
	struct timeval timeout;
	int rv;
	FD_ZERO(&set); /* clear the set */
	FD_SET(MainSocket, &set); /* add our file descriptor to the set */

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	rv = select(MainSocket + 1, &set, NULL, NULL, &timeout);
	if (rv == -1)
	{
	/* an error accured */
		return -1;
	}
	else if (rv == 0)
	{
	 /* a timeout occured */
		return 0;
	}
	else
		return 1;
	}

const char * Get_Message_Type(char *str) {
	char Message_Type[MESSAGE_TYPE_MAX_LENGTH];
	int c = 0;
	int str_len = strlen(str);
	while (c < str_len) {
		Message_Type[c] = str[c];
		c++;
		}
	Message_Type[c] = '\0';
	return Message_Type;
}

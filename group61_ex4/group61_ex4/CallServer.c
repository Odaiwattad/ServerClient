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
typedef struct Parameters_to_serviceThread {
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
	char Client_UserName[USERNAME_MAX_LENGTH];
	char Client_OpponentName[USERNAME_MAX_LENGTH];
	char Client_UserMove[MOVE_MAX_LENGTH];
	int Client_OpponentMove;
	int number_of_players;
	int Ready_to_MultiPlayer;
	int Last_played;
}Client_parameters;
static message message_details;
int Get_Num_of_Message_parameters(char *AcceptedStr, message *message_details);
void Init_Strings_parameters(message *message_details, int num_of_Params);
static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread(Client_parameters *Client_parm);
static DWORD Check_exit_server();
static int Accept_Socket(SOCKET MainSocket);
void CallServer();
char * number_to_name(int number);
int name_to_number(char *name);
int CPU_Vs_Client(char *player_choice, int CPU_Move);
TransferResult_t AnalayzeClientMessage(message * message_struct, Client_parameters *Client_parm,int CPU_Move);
TransferResult_t DealClientRequest(message * message_struct, Client_parameters *Client_Parm);
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/


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
	char SendStr[256];
	Client_parm.Ready_to_MultiPlayer = 0;
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
			strcpy(SendStr, SERVER_DENIED);
			strcat(SendStr,":Server disconnected\n");
		//	SendString(SendStr,
			goto server_cleanup_3; 
		}
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
	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	char *UserName = NULL;
	int CPU_Move;
	while (!Done)
	{
		char *AcceptedStr = NULL;
		message msg_struct;
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
			msg_struct = Get_Message_Details(AcceptedStr);
			printf("Message Type : %s\n", msg_struct.Message_Type);
			free(AcceptedStr);
		}
		if (msg_struct.Message_Type == CLIENT_DISCONNECT) {
			Done = TRUE;
		}
		CPU_Move = rand() % 5;
		SendRes =AnalayzeClientMessage(&msg_struct, Client_parm,CPU_Move);

		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			closesocket((Client_parm->ThreadInputs[Client_parm->number_of_players]));
			return 1;
		}
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

message Get_Message_Details(char *AcceptedStr) {
	message message_details;
	int c = 0, num_of_parameters,i=0;
	int str_len = strlen(AcceptedStr);
	char Char_To_Str[2];
	Char_To_Str[1] = '\0';
	num_of_parameters = Get_Num_of_Message_parameters(AcceptedStr,&message_details);
	Init_Strings_parameters(&message_details,num_of_parameters);
	while (AcceptedStr[c] != '\n') {
		while (AcceptedStr[c] != ':')
		{
			message_details.Message_Type[c] = AcceptedStr[c];
			if (AcceptedStr[c + 1]== ':' || AcceptedStr[c + 1] == '\n') {
				message_details.Message_Type[c + 1] = '\0';
				c++;
				break;
			}
			c++;
		}
		if (AcceptedStr[c] == '\n') { break; }
		c++;
		while (AcceptedStr[c] != ';') {
			Char_To_Str[0] = AcceptedStr[c];
			strcat(message_details.Message_parameters[i], Char_To_Str);
			if (AcceptedStr[c + 1] == ';' || AcceptedStr[c + 1] == '\n') {
				strcat(message_details.Message_parameters[i], "");
				c++;
				break;
			}
			c++;
		}
		i++;
		}

	return message_details;
}

int Get_Num_of_Message_parameters(char *AcceptedStr,message * message_details)//,message *message_details) {
{
	int i = 0, counter = 0, allocate_memory = 1;
	while (AcceptedStr[i]!= '\n') {
		allocate_memory++;
		if (AcceptedStr[i] == ':') {
			message_details->Message_Type = (char*)malloc(sizeof(char)*allocate_memory);
			allocate_memory = 1;
			counter++;
		}
		if (AcceptedStr[i] == ';') {
			message_details->Message_parameters[counter-1] = (char*)malloc(sizeof(char)*allocate_memory);
			allocate_memory = 1;
			counter++;
		}
		i++;
	}
	message_details->Message_parameters[counter - 1] = (char*)malloc(sizeof(char)*allocate_memory);
	return counter;
}

void Init_Strings_parameters(message * message_details,int num_of_Params) {
	int temp = num_of_Params, i = 0;
	while (temp != 0) {
		message_details->Message_parameters[i][0] ='\0';
		i++;
		temp--;
	}
	return;
}

char * number_to_name(int number)
{
	switch (number)
	{
	case 0: return "ROCK";

	case 1: return "SPOCK";

	case 2: return "PAPER";

	case 3: return "LIZARD";
		//scissors
	case 4: return "SCISSORS";

	default: return "Error";
	}
}

int name_to_number(char *name)
{
	if (strcmp(name, "ROCK") == 0)
		return 0;
	if (strcmp(name, "SPOCK") == 0)
		return 1;
	if (strcmp(name, "PAPER") == 0)
		return 2;
	if (strcmp(name, "LIZARD") == 0)
		return 3;
	if (strcmp(name, "SCISSORS") == 0)
		return 4;
	else
	{
		printf("Error");
		return 5;
	}

}

int CPU_Vs_Client(char *player_choice,int CPU_Move)
{
	int player_number = name_to_number(player_choice);
	int comp_number = CPU_Move;

	printf("\nPlayer chooses : %s\n", number_to_name(player_number));
	printf("Computer chooses : %s\n", number_to_name(comp_number));

	if (player_number == 5)
		return -1;
	else if (comp_number == player_number)
		return 0;
	else if ((comp_number - player_number) % 5 < 3)
		return 1;
	else
		return 2;

}


TransferResult_t DealClientRequest(message * message_struct, Client_parameters * Client_parm) {
	char SendStr[256];
	if (Client_parm->number_of_players < 2) {
		strcpy(Client_parm->Client_UserName, message_struct->Message_parameters[0]);
		strcpy(SendStr, SERVER_APPROVED);
		strcat(SendStr, "\n");
		SendString(SendStr, (Client_parm->ThreadInputs[Client_parm->number_of_players]));
		strcpy(SendStr, SERVER_MAIN_MENU);
		strcat(SendStr, "\n");
		return SendString(SendStr, (Client_parm->ThreadInputs[Client_parm->number_of_players]));
	}
	else
	{
		strcpy(SendStr, SERVER_DENIED);
		strcat(SendStr, ":Maximum players are reached\n");
		return SendString(SendStr,(Client_parm->ThreadInputs[Client_parm->number_of_players]));
	}
}
TransferResult_t DealClientMainMenu( Client_parameters * Client_parm) {
	return SendString(SERVER_MAIN_MENU, Client_parm->ThreadInputs[Client_parm->number_of_players]);
}
TransferResult_t DealClientCPU(message * message_struct, Client_parameters *Client_parm,int CPU_Move) {
	char SendStr[256];

	CPU_Move = rand() % 5;
	strcpy(Client_parm->Client_OpponentName, "Server");
	Client_parm->Client_OpponentMove = CPU_Move;
	strcpy(SendStr, SERVER_PLAYER_MOVE_REQUEST);
	strcat(SendStr, "\n");
	return SendString(SendStr,Client_parm->ThreadInputs[Client_parm->number_of_players]);
}

TransferResult_t DealClientPlayerMove(message * message_struct, Client_parameters * Client_parm, int CPU_Move) {
	char SendStr[256];
	int game_result;
	strcpy(Client_parm->Client_UserMove, message_struct->Message_parameters[0]);
	//massgeType:yourmove;OpponentUserName;OpponentMove;Winner\n
	if (Client_parm->Client_OpponentName!=NULL) {
		strcpy(SendStr, SERVER_GAME_RESULTS);
		strcat(SendStr, ":");
		strcat(SendStr, Client_parm->Client_UserMove);
		strcat(SendStr, ";");
		strcat(SendStr, Client_parm->Client_OpponentName);
		strcat(SendStr, ";");
		strcat(SendStr, number_to_name(Client_parm->Client_OpponentMove));
		game_result = CPU_Vs_Client(Client_parm->Client_UserMove, Client_parm->Client_OpponentMove);
		switch (game_result) {
		case -1:
			strcpy(SendStr, "Your choice is wrong, Try again\n");
			break;
		case 0:
			strcat(SendStr, "\n");
			break;
		case 1:
			strcat(SendStr, Client_parm->Client_OpponentName);
			break;
		case 2:
			strcat(SendStr, Client_parm->Client_UserName);
			strcat(SendStr, "\n");
			break;
		}
		SendString(SendStr, (Client_parm->ThreadInputs[Client_parm->number_of_players]));
		strcpy(SendStr, SERVER_GAME_OVER_MENU);
		strcat(SendStr, "\n");
		return SendString(SendStr, (Client_parm->ThreadInputs[Client_parm->number_of_players]));
	}
}

void GetOpponentNameMove(char * str,Client_parameters * Client_parm) {
	int i = 0, Name_Length = 0, Move_Length = 0;
	char *OpponentMove = NULL;
	while (str[i] != ' ') {
		Name_Length++;
		i++;
	}
	Name_Length++;
	Client_parm->Client_OpponentName[0] = '\0';
	strncat(Client_parm->Client_OpponentName, str, Name_Length);
	i++;
	while (str[i] != '\n') {
		Move_Length++;
		i++;
	}
	OpponentMove[0] = '\0';
	str += Name_Length;
	strncat(OpponentMove, str, Move_Length);
	Client_parm->Client_OpponentMove = name_to_number(OpponentMove);
	return;
}


TransferResult_t DealClientVersus(message * message_struct, Client_parameters * Client_parm,int CPU_Move){
	FILE * GameSession;
	char  SendStr[256];
	char * AcceptedStr = NULL;
	HANDLE mutex;
	DWORD wait_result;
	mutex = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */
	if (mutex == NULL)
	{
		printf("error creating Mutix\n : %d", GetLastError());
	}
	while (Client_parm->Ready_to_MultiPlayer!=1) {
		wait_result = WaitForSingleObject(mutex, INFINITE);
		GameSession = fopen("GameSession.txt", "r");
		if (GameSession == NULL) {// File doesn't exist
			GameSession = fopen("GameSession.txt", "w");
			strcpy(SendStr, SERVER_PLAYER_MOVE_REQUEST);
			strcat(SendStr, "\n");
			SendString(SendStr, Client_parm->ThreadInputs[Client_parm->number_of_players]);
			ReceiveString(&AcceptedStr, (Client_parm->ThreadInputs[Client_parm->number_of_players]));
			*message_struct = Get_Message_Details(AcceptedStr);
			strcpy(Client_parm->Client_UserMove, message_struct->Message_parameters[0]);
			fprintf(GameSession, "%s %s\n", Client_parm->Client_UserName, Client_parm->Client_UserMove);
			Client_parm->Ready_to_MultiPlayer = -1;//Client Ready to play = -1 ( first to open file)
			fclose(GameSession);
			//release Mutex 
			ReleaseMutex(mutex);
		}
		else if (Client_parm->Ready_to_MultiPlayer == 0) {//file exists and this is first time I try to open
			//hold mutex 
			strcpy(SendStr, SERVER_PLAYER_MOVE_REQUEST);
			strcat(SendStr, "\n");
			SendString(SendStr, Client_parm->ThreadInputs[Client_parm->number_of_players]);
			ReceiveString(&AcceptedStr, (Client_parm->ThreadInputs[Client_parm->number_of_players]));
			*message_struct = Get_Message_Details(AcceptedStr);
			strcpy(Client_parm->Client_UserMove, message_struct->Message_parameters[0]);
			if (fgets(SendStr, 256, GameSession) != NULL) {
				GetOpponentNameMove(SendStr, Client_parm);
				fclose(GameSession);
			}
			GameSession = fopen("GameSession.txt", "w");
			fprintf(GameSession, "%s %s\n", Client_parm->Client_UserName, Client_parm->Client_UserMove);
			fclose(GameSession);
			Client_parm->Ready_to_MultiPlayer = 1;
			ReleaseMutex(mutex);
			break;
		}
		else if (Client_parm->Ready_to_MultiPlayer == -1) {
			if (fgets(SendStr, 256, GameSession) != NULL) {
				if (fgets(SendStr, 256, GameSession) != NULL) {
					GetOpponentNameMove(SendStr, Client_parm);
					fclose(GameSession);
					Client_parm->Ready_to_MultiPlayer = 1;
					ReleaseMutex(mutex);
				}
			}

		}

		//TO DO Calculate game result and send to Client
	}

}

TransferResult_t AnalayzeClientMessage(message * message_struct, Client_parameters *Client_parm,int CPU_Move) {
//	int game_result;
	if (STRINGS_ARE_EQUAL(message_struct->Message_Type, CLIENT_REQUEST))///Here we must check if 2 Clients are already in and Deny The third Cliend 
	{                                                 /// number of clients Connected is Client_parm->number_of_players
		return DealClientRequest(message_struct, Client_parm);
	}
	else if (STRINGS_ARE_EQUAL(message_struct->Message_Type, CLIENT_MAIN_MENU)) 
	{
		return DealClientMainMenu(Client_parm);
	}
	else if (STRINGS_ARE_EQUAL(message_struct->Message_Type, CLIENT_CPU))
	{
		return DealClientCPU(message_struct, Client_parm,CPU_Move);
	}

	else if (STRINGS_ARE_EQUAL(message_struct->Message_Type, CLIENT_VERSUS))
	{
		return DealClientVersus(message_struct,Client_parm,CPU_Move);
	}
	else if (STRINGS_ARE_EQUAL(message_struct->Message_Type, CLIENT_PLAYER_MOVE))
	{
		return DealClientPlayerMove(message_struct, Client_parm, CPU_Move);
	}
	/*else if (STRINGS_ARE_EQUAL(message_struct->Message_Type, CLIENT_REPLAY))
	{
	//To DO
		return DealClientReplay(message_struct, Client_parm);
	}
*/


	
}

//if CLIENT_versus
//check for game session file
//if no file , create file and wait
//if found game session file , 
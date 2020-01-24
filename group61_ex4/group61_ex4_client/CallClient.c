

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

//#define TEST

#include <WinSock2.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "SharedHeaders.h"

#define BYTES_TO_READ_EACH_TIME 1

#pragma comment(lib,"ws2_32.lib") //Winsock Library

typedef struct Message
{
	char* main_message;
	char *parameters[MAX_NUM_OF_PARAMETERS];
} Message;


char* Username;
void TryToConnect(SOCKET soc, SOCKADDR_IN clientService, Message* message_struct);

void ToUppercase(char* string_to_change, int string_length)
{
	int i;
	char* temp_char_ptr = NULL;
	for (i = 0; i < string_length; i++)
	{
		temp_char_ptr = (string_to_change + i);


		*temp_char_ptr = toupper(*temp_char_ptr);

	}
}

void FreeMessageStruct(Message* message_struct)
{
	int parameter_index;
	if (message_struct == NULL)
	{
		return;
	}

	if (message_struct->main_message == NULL)
	{
		return;
	}
	else
	{
		free(message_struct->main_message);
	}

	for (parameter_index = 0; parameter_index < MAX_NUM_OF_PARAMETERS; parameter_index++)
	{
		if (message_struct->parameters[parameter_index] == NULL)
		{
			return;
		}
		else
		{
			free(message_struct->parameters[parameter_index]);
		}
	}
	return;
}

void DealMainMenu(SOCKET soc)
{
	int user_response;

	printf("Choose what to do next:\n");
	printf("1. Play against another client\n");
	printf("2. Play against the server\n");
	printf("3. View the leaderboard\n");

	scanf("%d", &user_response);

	switch (user_response)
	{
	case 1:
		send(soc, CLIENT_VERSUS, (int)strlen(CLIENT_VERSUS), 0);
		break;
	case 2:
		send(soc, CLIENT_CPU, (int)strlen(CLIENT_CPU), 0);
		break;
	case 3:
		send(soc, CLIENT_LEADERBOARD, (int)strlen(CLIENT_LEADERBOARD), 0);
		break;
	}
}

//TODO
void DealApproved(Message* message_struct, SOCKET soc)
{
	//char user_response;

	//no print

	//DEAL for stop counting seconds till show error

	//no scanf
}

void DealDenied(SOCKET soc, SOCKADDR_IN clientService, Message* message_struct, bool cant_connect, bool is_timed_out)
{
	char user_response;

	if (cant_connect)
	{
		printf("Failed connecting to server on %s:%d.\n", SERVER_ADDRESS_STR, SERVER_PORT);
	}
	else if (is_timed_out)
	{
		printf("Connection to server on %s:%d has been lost.\n", SERVER_ADDRESS_STR, SERVER_PORT);
	}
	else
	{
		printf("Server on %s:%d denied the connection request.\n", SERVER_ADDRESS_STR, SERVER_PORT);
	}
	printf("Choose what to do next:\n");
	printf("1. Try to reconnect\n");
	printf("2. Exit\n");



	scanf("%c", &user_response);

	if (user_response == '1')
	{
		TryToConnect(soc, clientService, message_struct);
	}
	else
	{
		FreeMessageStruct(message_struct);
		exit(1);
	}
}

//TODO
void DealInvite(Message* message_struct, SOCKET soc)
{
	//char user_response;

	// NO PRINT

	// DEAL

	//NO SCANF
}

void DealPlayerMoveRequest(SOCKET soc)
{
	char user_response[8];

	printf("Choose a move from the list: Rock, Paper, Scissors, Lizard or Spock:\n");

	scanf("%s", user_response);

	ToUppercase(user_response, 8);

	send(soc, user_response, 8, 0);
}

void DealGameResult(Message* message_struct)
{
	printf("You played: %s\n", message_struct->parameters[2]);
	printf("%s played: %s\n", message_struct->parameters[0], message_struct->parameters[1]);

	if (!(STRINGS_ARE_EQUAL(message_struct->parameters[2], message_struct->parameters[1])))
	{
		printf("%s won!\n", message_struct->parameters[3]);
	}
}

void DealGameOverMenu(SOCKET soc)
{
	int user_response;

	printf("Choose what to do next:\n");
	printf("1. Play again\n");
	printf("2. Return to the main menu\n");

	scanf("%d", &user_response);
	if (user_response == 1)
	{
		send(soc, CLIENT_REPLAY, (int)strlen(CLIENT_REPLAY), 0);
	}
	else
	{
		send(soc, CLIENT_MAIN_MENU, (int)strlen(CLIENT_MAIN_MENU), 0);
	}
}

void DealOpponentQuit(Message* message_struct)
{
	printf("%s has left the game!\n", message_struct->parameters[0]);
}

void DealNoOpponent(SOCKET soc)
{
	DealMainMenu(soc);
}

//TODO
void DealLeaderboard(Message* message_struct, SOCKET soc)
{
	//	char user_response;

		//TODO: prints...
		//printf("\n");

		//TODO: scanf
}

//TODO
void DealLeaderboardMenu(Message* message_struct, SOCKET soc)
{
	//	char user_response;

		//TODO: prints...
		//printf("\n");

		//TODO: scanf
}




bool CheckIfEndOfParameter(bool is_main_message, bool without_parameters, int parameter_index, char last_char)
{
	if (parameter_index == 3 || without_parameters)
	{
		return last_char == '\n';
	}
	else if (is_main_message)
	{
		return last_char == ':';
	}
	else
	{
		return last_char == ';';
	}
}

int GetMessageFromSocket(SOCKET sender_socket, bool without_parameters, Message* message_struct)  // return the length of the message
{
	char *parameters[MAX_NUM_OF_PARAMETERS];
	char* main_message = NULL;
	char** parameter_ptr = NULL;
	int bytes_readen_this_time;
	char buffer;
	int index = 0;
	int parameters_index;
	int index_within_parameter = 0;
#ifdef TEST
	FILE*  file = fopen("test.txt", "r");
#endif
	for (parameters_index = 0; parameters_index < MAX_NUM_OF_PARAMETERS; parameters_index++)
	{
		(parameters[parameters_index]) = NULL;
	}
	parameters_index = -1;

	do {
		if (index_within_parameter > 0)
		{
		}
		else if (parameters_index == -1)
		{
			parameter_ptr = &(main_message);
		}
		else
		{
			parameter_ptr = &(parameters[parameters_index]);
		}

		if (index_within_parameter == 0)
		{
			*parameter_ptr = (char*)malloc(sizeof(char));
		}
		else
		{
			*parameter_ptr = realloc(*parameter_ptr, (index_within_parameter + 1) * sizeof(char));
		}

#ifdef TEST
		buffer = fgetc(file);
#else
		bytes_readen_this_time = recv(sender_socket, &buffer, 1, 0);
		if (bytes_readen_this_time <= 0) {
			// free what need
			return false;
		}
#endif


		*((*parameter_ptr) + index_within_parameter) = buffer;


		if (CheckIfEndOfParameter(parameters_index == -1, without_parameters, parameters_index, buffer))
		{
			*((*parameter_ptr) + index_within_parameter) = '\0';
			index_within_parameter = 0;
			parameters_index++;
		}
		else
		{
			index_within_parameter++;
		}


	} while (buffer != '\n');
#ifdef TEST
	fclose(file);
#endif
	message_struct->main_message = main_message;
	for (parameters_index = 0; parameters_index < MAX_NUM_OF_PARAMETERS; parameters_index++)
	{
		message_struct->parameters[parameters_index] = parameters[parameters_index];
	}

	return true;
}

void AnalyzeServerMessage(Message* message_struct, SOCKET soc, SOCKADDR_IN clientService)
{
	char* main_message = message_struct->main_message;

	if (STRINGS_ARE_EQUAL(main_message, SERVER_MAIN_MENU))
	{
		DealMainMenu(soc);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_APPROVED))
	{
		DealApproved(message_struct, soc);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_DENIED))
	{
		DealDenied(soc, clientService, message_struct, false, false);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_INVITE))
	{
		DealInvite(message_struct, soc);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_PLAYER_MOVE_REQUEST))
	{
		DealPlayerMoveRequest(soc);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_GAME_RESULTS))
	{
		DealGameResult(message_struct);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_GAME_OVER_MENU))
	{
		DealGameOverMenu(soc);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_OPPONENT_QUIT))
	{
		DealOpponentQuit(message_struct);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_NO_OPPONENTS))
	{
		DealNoOpponent(soc);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_LEADERBOARD))
	{
		DealLeaderboard(message_struct, soc);
	}
	else if (STRINGS_ARE_EQUAL(main_message, SERVER_LEADERBOARD_MENU))
	{
		DealLeaderboardMenu(message_struct, soc);
	}
}

void TryToConnect(SOCKET soc, SOCKADDR_IN clientService, Message* message_struct)
{
	if (connect(soc, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed to connect.\n");
		WSACleanup();
		DealDenied(soc, clientService, message_struct, true, false);
	}
	else
	{
		send(soc, CLIENT_REQUEST, (int)strlen(CLIENT_REQUEST), 0);
		send(soc, Username, (int)strlen(Username), 0);
		send(soc, "\n", 1, 0);
	}
}

int CallClient()
{
	// Initialize Winsock.
	SOCKET m_socket;
	SOCKADDR_IN clientService;
	Username = "Odai";
	WSADATA wsaData;
	Message message_struct;
	/*
		char test[9];
		strcpy(test, "sCisSorS");
		ToUppercase(test, 8);*/

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		WSACleanup();
		return;
	}



	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); //Setting the IP address to connect to
	clientService.sin_port = htons(SERVER_PORT); //Setting the port to connect to.

	TryToConnect(m_socket, clientService, NULL);

	while (true)
	{
		GetMessageFromSocket(m_socket, false, &message_struct);

		AnalyzeServerMessage(&message_struct, m_socket, clientService);

		FreeMessageStruct(&message_struct);
	}
	//send(m_socket, CLIENT_REQUEST,strlen(CLIENT_REQUEST), 0);
	//send(m_socket, Username, strlen(Username), 0);
	//send(m_socket, "\n", 1, 0);

	return 0;
}

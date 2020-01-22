#ifndef SOCKET_EXAMPLE_SHARED_H
#define SOCKET_EXAMPLE_SHARED_H

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define SERVER_ADDRESS_STR "127.0.0.1"
#define SERVER_PORT 2345

#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
#define MESSAGE_TYPE_MAX_LENGTH 20
#define MAX_NUM_OF_PARAMETERS 6
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
//Server Massages 
#define SERVER_MAIN_MENU "Print Manu" 
#define SERVER_APPROVED "Connected"
#define SERVER_DENIED "Connection Denied"
#define SERVER_INVITE "Invite"
#define SERVER_PLAYER_MOVE_REQUEST "Choose Move"
#define SERVER_GAME_RESULTS "Game Results"
#define SERVER_GAME_OVER_MENU "Game Over Menu"
#define SERVER_OPPONENT_QUIT "Opponent Quit"
#define SERVER_NO_OPPONENTS "No Opponents"
#define SERVER_LEADERBOARD "Leaderboard"
#define SERVER_LEADERBOARD_MENU "Leaderboards Menu"

//Client Massages
#define CLIENT_REQUEST "Connect Request"
#define CLIENT_MAIN_MENU "Display Menu"
#define CLIENT_CPU "Client vs CPU"
#define CLIENT_VERSUS "Client vs Client"
#define CLIENT_LEADERBOARD "Leaderboards Menu"
#define CLIENT_PLAYER_MOVE "Client Choice"
#define CLIENT_REPLAY "Play Again"
#define CLIENT_REFRESH "Refresh Leaderboard"
#define CLIENT_DISCONNECT "Disconnect"

typedef struct Messages {
	char *Message_Type;
	char *Message_parameters[MAX_NUM_OF_PARAMETERS];
}message;


message Get_Message_Details(char *str);

#endif // SOCKET_EXAMPLE_SHARED_H

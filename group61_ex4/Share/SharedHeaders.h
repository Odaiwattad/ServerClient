#ifndef SOCKET_EXAMPLE_SHARED_H
#define SOCKET_EXAMPLE_SHARED_H

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define SERVER_ADDRESS_STR "127.0.0.1"
#define SERVER_PORT 2345

#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
#define MESSAGE_TYPE_MAX_LENGTH 20
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
//Server Massages 
#define SERVER_MAIN_MENU "Print Manu\n" 
#define SERVER_APPROVED "Connected\n"
#define SERVER_DENIED "Connection Denied\n"
#define SERVER_INVITE "Invite\n"
#define SERVER_PLAYER_MOVE_REQUEST "Choose Move\n"
#define SERVER_GAME_RESULTS "Game Results\n"
#define SERVER_GAME_OVER_MENU "Game Over Menu\n"
#define SERVER_OPPONENT_QUIT "Opponent Quit\n"
#define SERVER_NO_OPPONENTS "No Opponents\n"
#define SERVER_LEADERBOARD "Leaderboard\n"
#define SERVER_LEADERBOARD_MENU "Leaderboards Menu\n"

//Client Massages
#define CLIENT_REQUEST "Connect Request\n"
#define CLIENT_MAIN_MENU "Display Menu\n"
#define CLIENT_CPU "Client vs CPU\n"
#define CLIENT_VERSUS "Client vs Client\n"
#define CLIENT_LEADERBOARD "Leaderboards Menu\n"
#define CLIENT_PLAYER_MOVE "Client Choice\n"
#define CLIENT_REPLAY "Play Again\n"
#define CLIENT_REFRESH "Refresh Leaderboard\n"
#define CLIENT_DISCONNECT "Disconnect\n"

const char * Get_Message_Type(char *str);
#endif // SOCKET_EXAMPLE_SHARED_H

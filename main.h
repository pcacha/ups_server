#ifndef UPS_SERVER_MAIN_H
#define UPS_SERVER_MAIN_H

// sets up server on given port
void setupServer(int &serverSocket, struct sockaddr_in *myAddr);
// closes connection with client and delete the player from games and players
void closeConnection(int fd, fd_set *clientSockets);
// closes connection with client without delete
void closeConnectionWithoutRemovingPlayer(int fd, fd_set *clientSockets);
// main program
int main(int argc, char *argv[]);
// sends message to client
void send(int fd, string msg);
// determines if string is a number
bool stringIsNumber(string value);


#endif
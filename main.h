#ifndef UPS_SERVER_MAIN_H
#define UPS_SERVER_MAIN_H

// sets up server on given port
void setupServer(int &serverSocket, struct sockaddr_in *myAddr, int port);
// closes connection with client and delete the player from games and players
void closeConnection(int fd, fd_set *clientSockets);
// closes connection with client without delete
void closeConnectionWithoutRemovingPlayer(int fd, fd_set *clientSockets);
// main program
int main();
// sends message to client
void send(int fd, string msg);


#endif
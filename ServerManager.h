#ifndef UPS_SERVER_SERVERMANAGER_H
#define UPS_SERVER_SERVERMANAGER_H

#include <string>
#include <vector>
#include "Game.h"

using namespace std;

/*
 * Class represents serer manager that handles incoming messages
 */
class ServerManager {
public:
    // initial handling of message
    void handleMessage(int fd, string msg);
    // constructor
    explicit ServerManager(fd_set &_clientSockets);
    // disconect player from game
    void disconnectPlayer(int fd);
    // delete player's closed file descriptor
    void deletePlayersDescriptor(int fd);

private:
    // clients sockets
    fd_set &clientSockets;
    // list of all games
    vector<Game*> games;
    // list of all players
    vector<Player*> players;
    // handles connect message
    void handleConnect(int fd, vector<string> msgParts);
    // handles move messsage
    void handleMove(int fd, vector<string> msgParts);
    // finds player's game based on file descriptor id if the game is in te in game
    Game *findPlayersGameInGame(int fd);
    // chanes given coordinates to theirs inverse
    void makeCoordinatesInverse(int &sourceY, int &sourceX, int &goalY, int &goalX);
    // finds index of game in vector
    int findGameIdx(Game *game);
    // get player based on nick
    Player *getPlayerByNick(string nick);
    // handles leave message
    void handleLeave(int fd, vector <string> msgParts);
    // finds index of player in vector
    int findPlayerIdx(Player *player);
    // finds player's game based on file descriptor id
    Game *findPlayersGame(int fd);
    // finds player by file descriptor in vector
    Player *findPlayerByFd(int fd);
    // handles play again message
    void handlePlayAgain(int fd, vector <string> msgParts);
};


#endif

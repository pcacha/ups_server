#ifndef UPS_SERVER_PLAYER_H
#define UPS_SERVER_PLAYER_H

#include <string>
#include <chrono>

using namespace std;

/*
 * Class represents one player
 */
class Player {
public:
    // player's unique nick
    string nick;
    // player's file descriptor id
    int fd;
    // represents whether player is online
    bool online;
    // represents whether client exit unexpectedly
    bool terminated;
    // represents timestamp of last pong
    chrono::high_resolution_clock::time_point lastPongTimestamp;
    // constructor
    Player(string _nick, int _fd);
};


#endif

#ifndef UPS_SERVER_PLAYER_H
#define UPS_SERVER_PLAYER_H

#include <string>

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
    // represents whether player is active
    bool active;
    // constructor
    Player(string _nick, int _fd);
};


#endif

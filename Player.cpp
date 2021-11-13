#include "Player.h"
#include <string>

using namespace std;


Player::Player(string _nick, int _fd) {
    // player's unique nick
    nick = _nick;
    // player's file descriptor id
    fd = _fd;
    // set active
    active = true;
}
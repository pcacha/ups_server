#include "Player.h"
#include <string>
#include <chrono>

using namespace std;


Player::Player(string _nick, int _fd) {
    // player's unique nick
    nick = _nick;
    // player's file descriptor id
    fd = _fd;
    // set online
    online = true;
    // set terminated
    terminated = false;
    // set last pong
    lastPongTimestamp = chrono::high_resolution_clock::now();
}
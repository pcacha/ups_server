#include "ValidationUtils.h"
#include "Constants.h"
#include <string>

using namespace std;

bool ValidationUtils::validateNick(string nick){
    // nick has to be of specific length
    if(nick.length() == 0 || nick.length() > Constants::MAX_NAME_LENGTH)
    {
        return false;
    }

    // nick can not contain some chars
    if(nick.find(Constants::MSG_START) != string::npos || nick.find(Constants::MSG_END) != string::npos || nick.find(Constants::MSG_SEPARATOR) != string::npos || nick.find(Constants::MSG_NULL) != string::npos)
    {
        return false;
    }

    return true;
}

bool ValidationUtils::playerNotConnected(int fd, string nick, vector<Game*> &games) {
    // iterate all games
    for(int i = 0; i < games.size(); i++) {
        Game &game = *games[i];

        // game can not be in init state
        if(game.gameState != Game::GameState::INIT) {
            // check white player
            if(game.white->fd == fd || game.white->nick == nick) {
                return false;
            }

            // check black player
            if (game.gameState != Game::GameState::FIRST_CONNECTED) {
                if(game.black->fd == fd || game.black->nick == nick) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool ValidationUtils::socketNotUsed(int fd, vector<Player*> &players) {
    // iterate all players
    for(int i = 0; i < players.size(); i++) {
        Player &player = *players[i];

        // check socket
        if(player.fd == fd) {
            return false;
        }
    }

    return true;
}

bool ValidationUtils::validateMoveCoordinates(string sourceY, string sourceX, string goalY, string goalX) {
    string coordinates[] = {sourceY, sourceX, goalY, goalX};

    // iterate all coordinates
    for(int i = 0; i < 4; i++) {
        bool valid = false;

        // coordinates can be just from 0 - 7 range
        for(int j = 0; j < Constants::GAMEBOARD_LENGTH; j++) {
            string validString = to_string(j);

            if(coordinates[i] == validString) {
                valid = true;
                break;
            }
        }

        // if coordinate is not valid, return false
        if(!valid) {
            return false;
        }
    }

    return true;
}

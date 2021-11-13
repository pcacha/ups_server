#ifndef UPS_SERVER_VALIDATIONUTILS_H
#define UPS_SERVER_VALIDATIONUTILS_H


#include <string>
#include <vector>
#include "Game.h"

using namespace std;

/*
 * Class contains utility methods for validations
 */
class ValidationUtils {
public:
    // validates player's nick
    static bool validateNick(string nick);
    // checks if player is already connected
    static bool playerNotConnected(int fd, string nick, vector<Game*> &games);
    // checks if socket is already in use
    static bool socketNotUsed(int fd, vector<Player*> &players);
    // validates coordinates of player's move
    static bool validateMoveCoordinates(string sourceY, string sourceX, string goalY, string goalX);
};


#endif

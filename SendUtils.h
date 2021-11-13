#ifndef UPS_SERVER_SENDUTILS_H
#define UPS_SERVER_SENDUTILS_H


#include <string>

using namespace std;

/*
 * Class represents utility methods for sending messages to client
 */
class SendUtils {
public:
    // returns connect OK message
    static string connectOk(bool white);
    // returns connection invlaid message
    static string connectInvalid();
    // reutrns game message
    static string sendGame(string opponentNick, string playingNick, string winnerNick, string gameboardString);
    // returns move failed message
    static string moveFailed();
    // returns play again ok message
    static string playAgainOk(bool playerWhite);
};


#endif
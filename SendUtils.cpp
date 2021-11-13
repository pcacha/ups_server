#include "SendUtils.h"
#include "Constants.h"

using namespace std;

string SendUtils::connectOk(bool white) {
    string result = Constants::MSG_START + Constants::CONNECT + Constants::MSG_SEPARATOR + Constants::OK + Constants::MSG_SEPARATOR;
    if(white) {
        // player is white
        return result + Constants::PLAYER_WHITE + Constants::MSG_END;
    }
    // player is black
    return result + Constants::PLAYER_BLACK + Constants::MSG_END;
}

string SendUtils::connectInvalid() {
    return Constants::MSG_START + Constants::CONNECT + Constants::MSG_SEPARATOR + Constants::INVALID_NAME + Constants::MSG_END;
}

string SendUtils::sendGame(string opponentNick, string playingNick, string winnerNick, string gameboardString) {
    return Constants::MSG_START + Constants::GAME + Constants::MSG_SEPARATOR + opponentNick +
    Constants::MSG_SEPARATOR + playingNick + Constants::MSG_SEPARATOR + winnerNick + Constants::MSG_SEPARATOR + gameboardString + Constants::MSG_END;
}

string SendUtils::moveFailed() {
    return Constants::MSG_START + Constants::MOVE + Constants::MSG_SEPARATOR + Constants::FAILED + Constants::MSG_END;
}

string SendUtils::playAgainOk(bool playerWhite) {
    string result = Constants::MSG_START + Constants::PLAYAGAIN + Constants::MSG_SEPARATOR + Constants::OK + Constants::MSG_SEPARATOR;
    if(playerWhite) {
        // player is white
        return result + Constants::PLAYER_WHITE + Constants::MSG_END;
    }
    // player is black
    return result + Constants::PLAYER_BLACK + Constants::MSG_END;
}

string SendUtils::ping() {
    return Constants::MSG_START + Constants::PING + Constants::MSG_END;
}

string SendUtils::opponentOnline() {
    return Constants::MSG_START + Constants::OPPONENT_ONLINE + Constants::MSG_END;
}

string SendUtils::opponentOffline() {
    return Constants::MSG_START + Constants::OPPONENT_OFFLINE + Constants::MSG_END;
}

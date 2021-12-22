#ifndef UPS_SERVER_CONSTANTS_H
#define UPS_SERVER_CONSTANTS_H


#include "string"

using namespace std;

/*
 * Class containing all constants
 */
class Constants {
public:
    // enum describing move result
    enum MoveResult { INVALID, VALID, TAKED };
    // max length of protocol message
    static const int MAX_MSG_LENGTH = 110;
    // max length of messages waiting to be processed
    static const int MAX_MSG_BATCH_LENGTH = 500;
    // min length of protocol message
    static const int MIN_MSG_LENGTH = 4;
    // square game board length
    static const int GAMEBOARD_LENGTH = 8;
    // no descriptor assigned
    static const int EMPTY_FD = -1;
    // ping interval
    static const int PING_INTERVAL = 900;
    // static const int PING_INTERVAL = 40000;
    // max delay of pong [ms]
    static const int MAX_PONG_DELAY = 1500;
    // static const int MAX_PONG_DELAY = 60000;
    // max left time [ms] == 60 minutes
    static const int MAX_LEFT_TIME = 3600000;
    // statistical data file name
    static const string STAT_DATA_FILE_NAME;
    // message start indicator
    static const string MSG_START;
    // message end indicator
    static const string MSG_END;
    // separator of message parts
    static const string MSG_SEPARATOR;
    // message null indicator
    static const string MSG_NULL;
    // connect key word
    static const string CONNECT;
    // game key word
    static const string GAME;
    // OK key word
    static const string OK;
    // white key word
    static const string PLAYER_WHITE;
    // black key word
    static const string PLAYER_BLACK;
    // invalid name key word
    static const string INVALID_NAME;
    // move key word
    static const string MOVE;
    // failed key word
    static const string FAILED;
    // leave key word
    static const string LEAVE;
    // play again key word
    static const string PLAYAGAIN;
    // draw key word
    static const string DRAW;
    // ping key word
    static const string PING;
    // pong key word
    static const string PONG;
    // opponent online key word
    static const string OPPONENT_ONLINE;
    // opponent offline key word
    static const string OPPONENT_OFFLINE;
    // max length of player name
    static const int MAX_NAME_LENGTH = 10;
    // regex describing syntax of one message
    static const string MSG_REGEX;
    // code of empty field
    static const int EMPTY_FIELD = 1;
    // code of field with white stone
    static const int WHITE_STONE = 2;
    // code of field with black stone
    static const int BLACK_STONE = 3;
    // code of field with white king
    static const int WHITE_KING = 4;
    // code of field with black king
    static const int BLACK_KING = 5;
};


#endif

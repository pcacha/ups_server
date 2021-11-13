#include "Constants.h"
#include <string>

using namespace std;

const string Constants::MSG_START = "$";
const string Constants::MSG_END = "#";
const string Constants::MSG_SEPARATOR = "|";
const string Constants::MSG_NULL = "@";
const string Constants::CONNECT = "CONNECT";
const string Constants::GAME = "GAME";
const string Constants::OK = "OK";
const string Constants::PLAYER_WHITE = "WHITE";
const string Constants::PLAYER_BLACK = "BLACK";
const string Constants::INVALID_NAME = "INVALID_NAME";
const string Constants::MOVE = "MOVE";
const string Constants::FAILED = "FAILED";
const string Constants::LEAVE = "LEAVE";
const string Constants::PLAYAGAIN = "PLAYAGAIN";
const string Constants::DRAW = "@@";
const string Constants::MSG_REGEX = "\\" + MSG_START + "[^\\" + MSG_START + "\\" + MSG_END + "]*\\" + MSG_END;
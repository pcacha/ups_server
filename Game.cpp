#include "Game.h"
#include "Player.h"
#include "Constants.h"
#include "SendUtils.h"
#include "main.h"
#include <cstdlib>
#include <string>

using namespace std;

Game::Game() {
    white = nullptr;
    black = nullptr;;
    winner = nullptr;;
    playing = nullptr;;
    gameState = GameState::INIT;
    gameboard = new int*[Constants::GAMEBOARD_LENGTH];

    // creates game board 2D array
    for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
        gameboard[i] = new int[Constants::GAMEBOARD_LENGTH];
    }

    // initialize game board to start state
    for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
        for(int j = 0; j < Constants::GAMEBOARD_LENGTH; j++) {
            gameboard[i][j] = Constants::EMPTY_FIELD;

            if(isFieldBlack(i, j)) {
                // on black side
                if(i < 3) {
                    gameboard[i][j] = Constants::BLACK_STONE;
                }

                // on white side
                if(i > 4) {
                    gameboard[i][j] = Constants::WHITE_STONE;
                }
            }
        }
    }
}

Game::~Game() {
    // deletes game board 2D array
    for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
        delete gameboard[i];
    }
    delete gameboard;
}

bool Game::isFieldBlack(int i, int j) {
    if(!((i % 2 == 0 && j % 2 == 0) || (i % 2 != 0 && j % 2 != 0))){
        // field is black
        return true;
    }
    return false;
}

void Game::addFirstPlayer(Player *player) {
    // first player is always white and is on turn
    white = player;
    playing = white;
    gameState = GameState::FIRST_CONNECTED;
}
void Game::addSecondPlayer(Player *player) {
    // adds second player to game and change game state
    black = player;
    gameState = GameState::SECOND_CONNECTED;
}

void Game::sendGameToPlayers() {
    sendGameToPlayers(true, true);
}

void Game::sendGameToPlayers(bool sendWhite, bool sendBlack) {
    // playing player for message
    string playingNick = Constants::MSG_NULL;
    if(playing != nullptr) {
        playingNick = playing->nick;
    }

    // set winner if already exists
    string winnerNick = Constants::MSG_NULL;
    if(winner != nullptr) {
        winnerNick = winner->nick;
    }

    if(winner == nullptr && gameState == GameState::FINISHED) {
        winnerNick = Constants::DRAW;
    }

    // send game to both players
    if(sendWhite && white->fd != Constants::EMPTY_FD) {
        send(white->fd, SendUtils::sendGame(black->nick, playingNick, winnerNick, getGameboardString(gameboard)));
    }
    if(sendBlack && black->fd != Constants::EMPTY_FD) {
        send(black->fd, SendUtils::sendGame(white->nick, playingNick, winnerNick, getInverseGameboardString()));
    }
}

string Game::getInverseGameboardString() {
    // creates instance of inverse game board
    int **inverseGameboard = new int*[Constants::GAMEBOARD_LENGTH];

    for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
        inverseGameboard[i] = new int[Constants::GAMEBOARD_LENGTH];
    }

    // sets value of inverse game board
    for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
        for(int j = 0; j < Constants::GAMEBOARD_LENGTH; j++) {
            inverseGameboard[getInverseCoordinate(i)][getInverseCoordinate(j)] = gameboard[i][j];
        }
    }

    // creates encoded string of inverse game board
    string result = getGameboardString(inverseGameboard);

    // deletes help inverse array
    for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
        delete inverseGameboard[i];
    }
    delete inverseGameboard;

    return result;
}

string Game::getGameboardString(int **gameboard) {
    string result = "";

    // maps 2D array to string
    for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
        for(int j = 0; j < Constants::GAMEBOARD_LENGTH; j++) {
            result += to_string(gameboard[i][j]);
        }
    }

    return result;
}

int Game::getInverseCoordinate(int coordinate) {
    // counts inverse coordinate
    return abs(coordinate - (Constants::GAMEBOARD_LENGTH - 1));
}

bool Game::fieldEmpty(int y, int x) {
    // if field is empty return true
    if(gameboard[y][x] == Constants::EMPTY_FIELD) {
        return true;
    }

    return false;
}

bool Game::containsPlayersStone(int y, int x, bool playerWhite) {
    int fieldCode = gameboard[y][x];

    // for white player
    if(playerWhite && (fieldCode == Constants::WHITE_STONE || fieldCode == Constants::WHITE_KING)) {
        return true;
    }

    // for black player
    if(!playerWhite && (fieldCode == Constants::BLACK_STONE || fieldCode == Constants::BLACK_KING)) {
        return true;
    }

    return false;
}

bool Game::isFieldKing(int y, int x) {
    // if field has white or black king returns true
    if(gameboard[y][x] == Constants::BLACK_KING || gameboard[y][x] == Constants::WHITE_KING) {
        return true;
    }

    return false;
}

Constants::MoveResult Game::validateStoneMove(int sourceY, int sourceX, int goalY, int goalX, bool playerWhite) {
    int yDistinction = abs(sourceY - goalY);
    int xDistinction = abs(sourceX - goalX);

    // if move is not diagonal
    if(yDistinction != xDistinction) {
        return Constants::MoveResult::INVALID;
    }

    if(playerWhite) {
        // for move just one line forward
        if(sourceY - 1 == goalY && (sourceX == goalX + 1 || sourceX == goalX - 1)) {
            return Constants::MoveResult::VALID;
        }
        // if it moves two lines forward, it hase to take opponent's stone
        else if(sourceY - 2 == goalY && ((sourceX == goalX + 2 && containsPlayersStone(sourceY - 1, sourceX - 1, false)) ||
                                        (sourceX == goalX - 2 && containsPlayersStone(sourceY - 1, sourceX + 1, false)))) {
            return Constants::MoveResult::TAKED;
        }
        else {
            return Constants::MoveResult::INVALID;
        }
    }
    else {
        // for black player
        // for move just one line forward
        if(sourceY + 1 == goalY && (sourceX == goalX + 1 || sourceX == goalX - 1)) {
            return Constants::MoveResult::VALID;
        }
        // if it moves two lines forward, it hase to take opponent's stone
        else if(sourceY + 2 == goalY && ((sourceX == goalX + 2 && containsPlayersStone(sourceY + 1, sourceX - 1, true)) ||
                                         (sourceX == goalX - 2 && containsPlayersStone(sourceY + 1, sourceX + 1, true)))) {
            return Constants::MoveResult::TAKED;
        }
        else {
            return Constants::MoveResult::INVALID;
        }
    }
}

Constants::MoveResult Game::validateKingMove(int sourceY, int sourceX, int goalY, int goalX, bool kingWhite) {
    int yDistinction = abs(sourceY - goalY);
    int xDistinction = abs(sourceX - goalX);

    // move must be diagonal
    if(yDistinction != xDistinction) {
        return Constants::MoveResult::INVALID;
    }

    int yInc;
    int xInc;

    // count increment of y axis
    if(sourceY < goalY) {
        yInc = 1;
    }
    else {
        yInc = -1;
    }

    // count increment for x axis
    if(sourceX < goalX) {
        xInc = 1;
    }
    else {
        xInc = -1;
    }

    bool hasEnemy = false;
    int stonesInPath = 0;
    int yPos = sourceY + yInc;
    int xPos = sourceX + xInc;

    // for every coordinate between move start and move end
    while(yPos != goalY || xPos != goalX) {
        // counts stones on the path
        if(!fieldEmpty(yPos, xPos)) {
            stonesInPath++;
        }

        // if it is white king and field contains opponent's stone, take it
        if(kingWhite && containsPlayersStone(yPos, xPos, false)) {
            hasEnemy = true;
        }

        // if it is white black and field contains opponent's stone, take it
        if(!kingWhite && containsPlayersStone(yPos, xPos, true)) {
            hasEnemy = true;
        }

        // increment coordinates
        yPos += yInc;
        xPos += xInc;
    }

    // no stones in path
    if(stonesInPath == 0) {
        return Constants::MoveResult::VALID;
    }
    // one opponent's stone taked
    else if(stonesInPath == 1 && hasEnemy) {
        return Constants::MoveResult::TAKED;
    }
    else {
        return Constants::MoveResult::INVALID;
    }
}

bool Game::canJump(bool playerWhite) {
    if(playerWhite) {
        // for every game board field
        for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
            for(int j = 0; j < Constants::GAMEBOARD_LENGTH; j++) {
                if(gameboard[i][j] == Constants::WHITE_STONE && i >= 2) {
                    // if stone can take opponent's stone on right side
                    if(j <= 5 && containsPlayersStone(i - 1, j + 1, false) && fieldEmpty(i - 2, j + 2)) {
                        return true;
                    }
                    // if stone can take opponent's stone on left side
                    if(j >= 2 && containsPlayersStone(i - 1, j - 1, false) && fieldEmpty(i - 2, j - 2)) {
                        return true;
                    }
                }
                // if it is white king
                else if(gameboard[i][j] == Constants::WHITE_KING) {
                    if(canKingJump(i, j, true)) {
                        return true;
                    }
                }
            }
        }

        return false;
    }
    else {
        // for every game board field
        for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
            for(int j = 0; j < Constants::GAMEBOARD_LENGTH; j++) {
                if(gameboard[i][j] == Constants::BLACK_STONE && i <= 5) {
                    // if stone can take opponent's stone on right side
                    if(j <= 5 && containsPlayersStone(i + 1, j + 1, true) && fieldEmpty(i + 2, j + 2)) {
                        return true;
                    }
                    // if stone can take opponent's stone on left side
                    if(j >= 2 && containsPlayersStone(i + 1, j - 1, true) && fieldEmpty(i + 2, j - 2)) {
                        return true;
                    }
                }
                // if it is black king
                else if(gameboard[i][j] == Constants::BLACK_KING) {
                    if(canKingJump(i, j, false)) {
                        return true;
                    }
                }
            }
        }

        return false;
    }
}

void Game::setFieldEmpty(int y, int x) {
    gameboard[y][x] = Constants::EMPTY_FIELD;
}

void Game::setFieldStone(int y, int x, bool playerWhite) {
    if(playerWhite) {
        // set white stone
        gameboard[y][x] = Constants::WHITE_STONE;
    }
    else {
        // set black stone
        gameboard[y][x] = Constants::BLACK_STONE;
    }
}

void Game::setFieldKing(int y, int x, bool playerWhite) {
    if(playerWhite) {
        // set white king
        gameboard[y][x] = Constants::WHITE_KING;
    }
    else {
        // set black king
        gameboard[y][x] = Constants::BLACK_KING;
    }
}

void Game::tryUpgradeToKing(int y, int x, bool playerWhite) {
    if(playerWhite) {
        // if white stone is at the end of game board it becomes king
        if(y == 0) {
            gameboard[y][x] = Constants::WHITE_KING;
        }
    }
    else {
        // if black stone is at the end of game board it becomes king
        if(y == Constants::GAMEBOARD_LENGTH - 1) {
            gameboard[y][x] = Constants::BLACK_KING;
        }
    }
}

bool Game::canKingJump(int y, int x, bool kingWhite) {
    int j;

    // diagonal - right down
    if(y <= 5 && x <= 5) {
        j = x + 1;
        // for every field in the direction
        for(int i = y + 1; i < Constants::GAMEBOARD_LENGTH - 1; i++) {
            if(j >= Constants::GAMEBOARD_LENGTH - 1) {
                break;
            }

            // if there is opponent's stone in path, there has to be empty field next
            if(((kingWhite && containsPlayersStone(i, j, false)) || (!kingWhite && containsPlayersStone(i, j, true)) && fieldEmpty(i + 1, j + 1))) {
                return true;
            }
            else if(!fieldEmpty(i, j)) {
                break;
            }

            j++;
        }
    }

    // diagonal - left down
    if(y <= 5 && x >= 2) {
        j = x - 1;
        // for every field in the direction
        for(int i = y + 1; i < Constants::GAMEBOARD_LENGTH - 1; i++) {
            if(j <= 0) {
                break;
            }

            // if there is opponent's stone in path, there has to be empty field next
            if(((kingWhite && containsPlayersStone(i, j, false)) || (!kingWhite && containsPlayersStone(i, j, true)) && fieldEmpty(i + 1, j - 1))) {
                return true;
            }
            else if(!fieldEmpty(i, j)) {
                break;
            }

            j--;
        }
    }

    // diagonal - right up
    if(y >= 2 && x <= 5) {
        j = x + 1;
        // for every field in the direction
        for(int i = y - 1; i > 0; i--) {
            if(j >= Constants::GAMEBOARD_LENGTH - 1) {
                break;
            }

            // if there is opponent's stone in path, there has to be empty field next
            if(((kingWhite && containsPlayersStone(i, j, false)) || (!kingWhite && containsPlayersStone(i, j, true)) && fieldEmpty(i - 1, j + 1))) {
                return true;
            }
            else if(!fieldEmpty(i, j)) {
                break;
            }

            j++;
        }
    }

    // diagonal - left up
    if(y >= 2 && x >= 2) {
        j = x - 1;
        // for every field in the direction
        for(int i = y - 1; i > 0; i--) {
            if(j <= 0) {
                break;
            }

            // if there is opponent's stone in path, there has to be empty field next
            if(((kingWhite && containsPlayersStone(i, j, false)) || (!kingWhite && containsPlayersStone(i, j, true)) && fieldEmpty(i - 1, j - 1))) {
                return true;
            }
            else if(!fieldEmpty(i, j)) {
                break;
            }

            j--;
        }
    }

    return false;
}

void Game::setFieldRangeEmpty(int sourceY, int sourceX, int goalY, int goalX) {
    int yInc;
    int xInc;

    // increment for y axis
    if(sourceY < goalY) {
        yInc = 1;
    }
    else {
        yInc = -1;
    }

    // increment for x axis
    if(sourceX < goalX) {
        xInc = 1;
    }
    else {
        xInc = -1;
    }

    int yPos = sourceY + yInc;
    int xPos = sourceX + xInc;

    while(yPos != goalY || xPos != goalX) {
        // set every field in path empty
        setFieldEmpty(yPos, xPos);

        yPos += yInc;
        xPos += xInc;
    }
}

bool Game::playerWin(bool playerWhite) {
    int blackStonesCount = 0;
    int whiteStonesCount = 0;

    // for every field in game board
    for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
        for(int j = 0; j < Constants::GAMEBOARD_LENGTH; j++) {
            int fieldCode = gameboard[i][j];

            // count white players stones
            if(fieldCode == Constants::WHITE_STONE || fieldCode == Constants::WHITE_KING) {
                whiteStonesCount++;
            }
            // count black players stones
            else if(fieldCode == Constants::BLACK_STONE || fieldCode == Constants::BLACK_KING) {
                blackStonesCount++;
            }
        }
    }

    if(playerWhite) {
        return blackStonesCount == 0;
    }
    else {
        return whiteStonesCount == 0;
    }
}

bool Game::isDraw() {
    int blackKings = 0;
    int whiteKings = 0;
    int blackStones = 0;
    int whiteStones = 0;

    // for every field in game board
    for(int i = 0; i < Constants::GAMEBOARD_LENGTH; i++) {
        for(int j = 0; j < Constants::GAMEBOARD_LENGTH; j++) {
            int fieldCode = gameboard[i][j];

            // count kings
            if(fieldCode == Constants::WHITE_KING) {
                whiteKings++;
            }
            else if(fieldCode == Constants::BLACK_KING) {
                blackKings++;
            }
            else if(fieldCode == Constants::WHITE_STONE) {
                whiteStones++;
            }
            else if(fieldCode == Constants::BLACK_STONE) {
                blackStones++;
            }
        }
    }

    // if there are just two kings of different colour -> draw
    if(blackKings == 1 && whiteKings == 1 && whiteStones == 0 && blackStones == 0) {
        return true;
    }

    return false;
}

bool Game::canJumpFrom(int y, int x, bool playerWhite) {
    // if stone is king
    if(isFieldKing(y, x)) {
        return canKingJump(y, x, playerWhite);
    }
    else {
        // for white stone
        if(playerWhite) {
            if(y >= 2) {
                // take on right side
                if(x <= 5 && containsPlayersStone(y - 1, x + 1, false) && fieldEmpty(y - 2, x + 2)) {
                    return true;
                }
                // take on left side
                if(x >= 2 && containsPlayersStone(y - 1, x - 1, false) && fieldEmpty(y - 2, x - 2)) {
                    return true;
                }
            }
        }
        // for black stone
        else {
            if(y <= 5) {
                // take on right side
                if(x <= 5 && containsPlayersStone(y + 1, x + 1, true) && fieldEmpty(y + 2, x + 2)) {
                    return true;
                }
                // take on left side
                if(x >= 2 && containsPlayersStone(y + 1, x - 1, true) && fieldEmpty(y + 2, x - 2)) {
                    return true;
                }
            }
        }
    }
    return false;
}

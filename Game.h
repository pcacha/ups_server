#ifndef UPS_SERVER_GAME_H
#define UPS_SERVER_GAME_H


#include "Player.h"
#include "Constants.h"

/*
 * Class representing one room/game
 */
class Game {
public:
    // enum representing current state of game
    enum GameState { INIT, FIRST_CONNECTED, SECOND_CONNECTED, IN_GAME, FINISHED };
    // instance of white player
    Player *white;
    // instance of black player
    Player *black;
    // winner of the game
    Player *winner;
    // player who is on turn
    Player *playing;
    // state of the game
    GameState gameState;
    // 2D game board encoded with integers of 1 - 5
    int **gameboard;
    // constructor
    Game();
    // destructor
    ~Game();
    // method indicating whether passed field is black
    bool isFieldBlack(int i, int j);
    // adds first (white) player to game
    void addFirstPlayer(Player *player);
    // adds second (black) player to game
    void addSecondPlayer(Player *player);
    // sends state of game to players
    void sendGameToPlayers();
    // sends state of game to selected players
    void sendGameToPlayers(bool sendWhite, bool sendBlack);
    // gets game board encoded in string for black player with inverse game board
    string getInverseGameboardString();
    // gets game board encoded in string
    string getGameboardString(int **gameboard);
    // returns inverse coordinate for given coordinate
    static int getInverseCoordinate(int coordinate);
    // indicates whether given field is empty
    bool fieldEmpty(int y, int x);
    // indicates whether field contains stone or king of player with given color
    bool containsPlayersStone(int y, int x, bool playerWhite);
    // indicates whether field contains black white king
    bool isFieldKing(int y, int x);
    // validates move of player of given color
    Constants::MoveResult validateStoneMove(int sourceY, int sourceX, int goalY, int goalX, bool playerWhite);
    // sets field empty
    void setFieldEmpty(int y, int x);
    // sets stone of given color on given field
    void setFieldStone(int y, int x, bool playerWhite);
    // trys to upgrade stone to king when it reaches end of game board
    void tryUpgradeToKing(int y, int x, bool playerWhite);
    // returns whether player of given color can take enemy stone with his stone
    bool canJump(bool playerWhite);
    // return whether king with given coordinates and color can take opponent's stone
    bool canKingJump(int y, int x, bool kingWhite);
    // validates king move and returns its validity, it does not move stones around the game board
    Constants::MoveResult validateKingMove(int sourceY, int sourceX, int goalY, int goalX, bool playerWhite);
    // sets king of given color on given field
    void setFieldKing(int y, int x, bool playerWhite);
    // reaction to king's move - it takes opponent's stone in the path
    void setFieldRangeEmpty(int sourceY, int sourceX, int goalY, int goalX);
    // returns whether player who was on turn wins
    bool playerWin(bool playerWhite);
    // returns whether game ends as draw
    bool isDraw();
    // returns whether stone on given coordinates can take opponent's stone
    bool canJumpFrom(int y, int x, bool playerWhite);
};


#endif
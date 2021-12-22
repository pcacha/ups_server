#include <string>
#include "ServerManager.h"
#include "main.h"
#include "ValidationUtils.h"
#include "Constants.h"
#include "StringUtils.h"
#include "SendUtils.h"
#include <vector>
#include <iostream>
#include "Game.h"
#include "Player.h"
#include <chrono>

using namespace std;

ServerManager::ServerManager(fd_set &_clientSockets) : clientSockets(_clientSockets) {
}

void ServerManager::handleMessage(int fd, string msg) {
    // removes start and stop chars
    msg = msg.substr(1, msg.length() - 2);

    // splits message parts
    vector<string> msgParts = StringUtils::split(msg, Constants::MSG_SEPARATOR[0]);

    if(msgParts.empty()) {
        cout << "Message - empty message parts" << endl;
        closeConnection(fd, &clientSockets);
    }

    string command = msgParts[0];
    // find appropriate handler
    if(command == Constants::CONNECT) {
        cout << "Handle - connect" << endl;
        handleConnect(fd, msgParts);
    }
    else if(command == Constants::MOVE) {
        cout << "Handle - move" << endl;
        handleMove(fd, msgParts);
    }
    else if(command == Constants::LEAVE) {
        cout << "Handle - leave" << endl;
        handleLeave(fd, msgParts);
    }
    else if(command == Constants::PLAYAGAIN) {
        cout << "Handle - play again" << endl;
        handlePlayAgain(fd, msgParts);
    }
    else if(command == Constants::PONG) {
        handlePong(fd, msgParts);
    }
    else {
        cout << "Message - invalid command keyword" << endl;
        closeConnection(fd, &clientSockets);;
    }
}

void ServerManager::handleConnect(int fd, vector<string> msgParts) {
    if(msgParts.size() != 2 || !ValidationUtils::socketNotUsed(fd, players)) {
        // if count of message parts is wrong close connection and validate already connected sockets
        cout << "Connect - invalid message parts count or socket is already connected" << endl;
        closeConnection(fd, &clientSockets);
        return;
    }

    string nick = msgParts[1];

    // validate rules for player's nick
    if(!ValidationUtils::validateNick(nick)) {
        cout << "Connect - invalid nick" << endl;
        send(fd, SendUtils::connectInvalid());
    }
    else {
        // find player with this nick
        Player *alreadyConnectedPlayer = getPlayerByNick(nick);

        // no player is using this nick
        if(alreadyConnectedPlayer == nullptr) {
            cout << "Connect - no player is using this nick" << endl;
            // if player is first in room
            if(games.empty() || games.back()->gameState != Game::GameState::FIRST_CONNECTED) {
                cout << "Connect - player first in game" << endl;
                // create new game
                Game *game = new Game();
                games.push_back(game);
                // create player
                Player *player = new Player(nick, fd);
                players.push_back(player);
                // add player
                game->addFirstPlayer(player);
                send(fd, SendUtils::connectOk(true));
            }
            else {
                cout << "Connect - player second in game" << endl;
                // create player
                Player *player = new Player(nick, fd);
                players.push_back(player);
                // add player to last room
                Game *game = games.back();
                game->addSecondPlayer(player);
                send(fd, SendUtils::connectOk(false));

                if(!game->white->online) {
                    // if opponent is offline, send it
                    send(fd, SendUtils::opponentOffline());
                }

                // start game
                game->sendGameToPlayers();
                game->gameState = Game::GameState::IN_GAME;
            }
        }
        else {
            // player with this nick is registered
            if(alreadyConnectedPlayer->terminated) {
                // reconnect
                cout << "Reconnect - player reconnected" << endl;
                alreadyConnectedPlayer->terminated = false;
                alreadyConnectedPlayer->fd = fd;

                Game *game = findPlayersGame(fd);

                if(game == nullptr) {
                    // reconnected player not in game
                    // if player is first in room
                    if(games.empty() || games.back()->gameState != Game::GameState::FIRST_CONNECTED) {
                        cout << "Reconnect - player first in game" << endl;
                        // create new game
                        Game *game = new Game();
                        games.push_back(game);
                        // add player
                        game->addFirstPlayer(alreadyConnectedPlayer);
                        send(fd, SendUtils::connectOk(true));
                    }
                    else {
                        cout << "Reconnect - player second in game" << endl;
                        // add player to last room
                        Game *game = games.back();
                        game->addSecondPlayer(alreadyConnectedPlayer);
                        send(fd, SendUtils::connectOk(false));

                        if(!game->white->online) {
                            // if opponent is offline, send it
                            send(fd, SendUtils::opponentOffline());
                        }

                        // start game
                        game->sendGameToPlayers();
                        game->gameState = Game::GameState::IN_GAME;
                    }
                }
                else {
                    if(game->gameState != Game::GameState::IN_GAME) {
                        // player in queue
                        cout << "Reconnect - player joined queue again" << endl;
                        send(fd, SendUtils::connectOk(true));
                    }
                    else {
                        if(game->white->fd == fd) {
                            // if player is white
                            cout << "Reconnect - player joined game as white" << endl;
                            send(fd, SendUtils::connectOk(true));
                        }
                        else {
                            //  if player is black
                            cout << "Reconnect - player joined game as black" << endl;
                            send(fd, SendUtils::connectOk(false));
                        }

                        game->sendGameToPlayers();
                    }
                }
            }
            else {
                // the nick is already used, connect failed
                cout << "Connect - invalid - nick is already used" << endl;
                send(fd, SendUtils::connectInvalid());
            }
        }
    }
}

void ServerManager::handleMove(int fd, vector<string> msgParts) {
    // validate count of parts and passed coordinates
    if(msgParts.size() != 5 || !ValidationUtils::validateMoveCoordinates(msgParts[1], msgParts[2], msgParts[3], msgParts[4])) {
        cout << "Move - invalid message parts count or bad coordinates" << endl;
        closeConnection(fd, &clientSockets);
        return;
    }

    // parse coordinates
    int sourceY = stoi(msgParts[1]);
    int sourceX = stoi(msgParts[2]);
    int goalY = stoi(msgParts[3]);
    int goalX = stoi(msgParts[4]);

    // find player's game
    Game *game = findPlayersGameInGame(fd);
    if(game == nullptr) {
        cout << "Move - player is not in game" << endl;
        closeConnection(fd, &clientSockets);
        return;
    }

    bool playerWhite;
    Player *player;
    Player *opponent;

    // set up properties for white player
    if(game->white->fd == fd) {
        cout << "Move - player white" << endl;
        playerWhite = true;
        player = game->white;
        opponent = game->black;
    }
    // set up properties for black player
    else {
        cout << "Move - player black" << endl;
        playerWhite = false;
        player = game->black;
        opponent = game->white;
        // make coordinates inverse
        // makeCoordinatesInverse(sourceY, sourceX, goalY, goalX);
    }

    // it has to be player's turn
    if(game->playing != player) {
        cout << "Move - player is not on turn" << endl;
        closeConnection(fd, &clientSockets);
        return;
    }

    // source field must contain players stone and goal field has to be empty
    if(!game->containsPlayersStone(sourceY, sourceX, playerWhite) || !game->fieldEmpty(goalY, goalX) || (sourceX == goalX && sourceY == goalY)) {
        cout << "Move - invalid move - source not containing players stone, goal is not empty or goal and source are the same" << endl;
        send(fd, SendUtils::moveFailed());
        return;
    }

    bool isKing = game->isFieldKing(sourceY, sourceX);
    Constants::MoveResult moveResult;

    if(!isKing) {
        cout << "Move - source - stone" << endl;
        // if source field is stone

        // validate move
        moveResult = game->validateStoneMove(sourceY, sourceX, goalY, goalX, playerWhite);

        if(moveResult == Constants::MoveResult::INVALID) {
            // invalid move
            cout << "Move - move is not valid" << endl;
            send(fd, SendUtils::moveFailed());
            return;
        }

        if(moveResult == Constants::MoveResult::TAKED) {
            cout << "Move - opponent's stone taked" << endl;
            // if opponent's stone was taked, move source stone and delete taked stone
            game->setFieldEmpty(sourceY, sourceX);
            game->setFieldEmpty((sourceY + goalY) / 2, (sourceX + goalX) / 2);
            game->setFieldStone(goalY, goalX, playerWhite);
        }
        else {
            // if player has to take opponent's stone and didn't do it, move is invalid
            if(game->canJump(playerWhite)) {
                cout << "Move - invalid move - player must take opponent's stone" << endl;
                send(fd, SendUtils::moveFailed());
                return;
            }

            cout << "Move - move valid" << endl;
            // move source field
            game->setFieldEmpty(sourceY, sourceX);
            game->setFieldStone(goalY, goalX, playerWhite);
        }

        // if stone is at the end of game board, it becomes king
        game->tryUpgradeToKing(goalY, goalX, playerWhite);
    }
    else {
        cout << "Move - source - king" << endl;
        // for king

        // validate move
        moveResult = game->validateKingMove(sourceY, sourceX, goalY, goalX, playerWhite);

        if(moveResult == Constants::MoveResult::INVALID) {
            cout << "Move - move is not valid" << endl;
            // for invalid move
            send(fd, SendUtils::moveFailed());
            return;
        }

        if(moveResult == Constants::MoveResult::TAKED) {
            cout << "Move - opponent's stone taked" << endl;
            // if opponent's stone was taked, move source stone and delete taked stone
            game->setFieldEmpty(sourceY, sourceX);
            game->setFieldRangeEmpty(sourceY, sourceX, goalY, goalX);
            game->setFieldKing(goalY, goalX, playerWhite);
        }
        else {
            // if player has to take opponent's stone and didn't do it, move is invalid
            if(game->canJump(playerWhite)) {
                cout << "Move - invalid move - player must take opponent's stone" << endl;
                send(fd, SendUtils::moveFailed());
                return;
            }

            cout << "Move - move valid" << endl;
            // move source field
            game->setFieldEmpty(sourceY, sourceX);
            game->setFieldKing(goalY, goalX, playerWhite);
        }
    }

    cout << "Move - " << player->nick << " - [" << sourceY << "][" << sourceX << "] -> [" << goalY << "][" << goalX << "]" << endl;
    if(game->playerWin(playerWhite)) {
        cout << "Move - game end - player is the winner" << endl;
        // if player win set game and send message to players
        game->winner = player;
        game->playing = nullptr;
        game->gameState = Game::GameState::FINISHED;
        game->sendGameToPlayers();

        cout << "Move - deleting game" << endl;
        // remove game from vector
        games.erase(games.begin() + findGameIdx(game));
        delete game;
    }
    else if(game->isDraw()) {
        cout << "Move - game end - draw" << endl;
        // if it is draw set game and send message to players
        game->winner = nullptr;
        game->playing = nullptr;
        game->gameState = Game::GameState::FINISHED;
        game->sendGameToPlayers();

        cout << "Move - deleting game" << endl;
        // remove game from vector
        games.erase(games.begin() + findGameIdx(game));
        delete game;
    }
    else {
        // if player taked opponent's stone and still can jump -> has turn again
        if(moveResult == Constants::MoveResult::TAKED && game->canJumpFrom(goalY, goalX, playerWhite)) {
            cout << "Move - player has another turn" << endl;
            game->playing = player;
        }
        else {
            cout << "Move - opponent's turn" << endl;
            game->playing = opponent;
        }
        game->sendGameToPlayers();
    }
}

Game *ServerManager::findPlayersGameInGame(int fd) {
    // iterate all games
    for(int i = 0; i < games.size(); i++) {
        Game *game = games[i];

        // if game has state in game and one of players has the file descriptor id, return the game
        if(game->gameState == Game::GameState::IN_GAME) {
            if(game->white->fd == fd || game->black->fd == fd) {
                return game;
            }
        }
    }

    return nullptr;
}

void ServerManager::makeCoordinatesInverse(int &sourceY, int &sourceX, int &goalY, int &goalX) {
    // make all passed coordinates inverse
    sourceY = Game::getInverseCoordinate(sourceY);
    sourceX = Game::getInverseCoordinate(sourceX);
    goalY = Game::getInverseCoordinate(goalY);
    goalX = Game::getInverseCoordinate(goalX);
}

int ServerManager::findGameIdx(Game *game) {
    // iterate all games and find index of passed game
    for(int i = 0; i < games.size(); i++) {
        if(game == games[i]) {
            return i;
        }
    }

    return -1;
}

Player *ServerManager::getPlayerByNick(string nick) {
    // iterate all players
    for(int i = 0; i < players.size(); i++) {
        Player *player = players[i];

        // check name, if it is the same return player
        if(player->nick == nick) {
            return player;
        }
    }

    return nullptr;
}

void ServerManager::handleLeave(int fd, vector<string> msgParts) {
    if(msgParts.size() != 1) {
        // if count of message parts is wrong close connection
        cout << "Leave - invalid message parts count" << endl;
        closeConnection(fd, &clientSockets);
        return;
    }

    // find player's game
    Game *game = findPlayersGame(fd);
    if(game == nullptr) {
        Player *player = findPlayerByFd(fd);

        if(player == nullptr) {
            cout << "Leave - player is not in game" << endl;
            closeConnection(fd, &clientSockets);
            return;
        }
        else {
            cout << "Leave - player deleted from players" << endl;
            players.erase(players.begin() + findPlayerIdx(player));
            delete player;
            return;
        }
    }

    // for white player
    if(game->white->fd == fd) {
        cout << "Leave - player white" << endl;

        // setup game and send it to players
        if(game->black != nullptr) {
            cout << "Leave - game end - player black is the winner" << endl;
            game->winner = game->black;
            game->playing = nullptr;
            game->gameState = Game::GameState::FINISHED;
            game->sendGameToPlayers(false, true);
        }

        cout << "Leave - deleting player" << endl;
        // remove player from vector
        players.erase(players.begin() + findPlayerIdx(game->white));
        delete game->white;
    }
    else {
        // for black player
        cout << "Leave - player black" << endl;

        // setup game and send it to players
        if(game->white != nullptr) {
            cout << "Leave - game end - player white is the winner" << endl;
            game->winner = game->white;
            game->playing = nullptr;
            game->gameState = Game::GameState::FINISHED;
            game->sendGameToPlayers(true, false);
        }

        cout << "Leave - deleting player" << endl;
        // remove player from vector
        players.erase(players.begin() + findPlayerIdx(game->black));
        delete game->black;
    }

    cout << "Leave - deleting game" << endl;
    // remove game from vector
    games.erase(games.begin() + findGameIdx(game));
    delete game;
}

void ServerManager::disconnectPlayer(int fd) {
    cout << "Disconnect - player has done something bad - trying to delete him from games and players" << endl;
    // find player's game
    Game *game = findPlayersGame(fd);
    if(game == nullptr) {
        Player *player = findPlayerByFd(fd);

        // if player exists
        if(player != nullptr) {
            cout << "Disconnect - player deleted from players" << endl;
            players.erase(players.begin() + findPlayerIdx(player));
            delete player;
        }

        return;
    }

    // for white player
    if(game->white->fd == fd) {
        cout << "Disconnected - player white" << endl;

        // setup game and send it to players
        if(game->black != nullptr) {
            cout << "Disconnected - game end - player black is the winner" << endl;
            game->winner = game->black;
            game->playing = nullptr;
            game->gameState = Game::GameState::FINISHED;
            game->sendGameToPlayers(false, true);
        }

        cout << "Disconnected - deleting player" << endl;
        // remove player from vector
        players.erase(players.begin() + findPlayerIdx(game->white));
        delete game->white;
    }
    else {
        // for black player
        cout << "Disconnected - player black" << endl;

        // setup game and send it to players
        if(game->white != nullptr) {
            cout << "Disconnected - game end - player white is the winner" << endl;
            game->winner = game->white;
            game->playing = nullptr;
            game->gameState = Game::GameState::FINISHED;
            game->sendGameToPlayers(true, false);
        }

        cout << "Disconnected - deleting player" << endl;
        // remove player from vector
        players.erase(players.begin() + findPlayerIdx(game->black));
        delete game->black;
    }

    cout << "Disconnected - deleting game" << endl;
    // remove game from vector
    games.erase(games.begin() + findGameIdx(game));
    delete game;
}

int ServerManager::findPlayerIdx(Player *player) {
    // iterate all players and find index of passed player
    for(int i = 0; i < players.size(); i++) {
        if(player == players[i]) {
            return i;
        }
    }

    return -1;
}

Game *ServerManager::findPlayersGame(int fd) {
    // iterate all games
    for(int i = 0; i < games.size(); i++) {
        Game *game = games[i];

        // if one of players has the file descriptor id, return the game
        if((game->white != nullptr && game->white->fd == fd) || (game->black != nullptr && game->black->fd == fd)) {
            return game;
        }
    }

    return nullptr;
}

Player *ServerManager::findPlayerByFd(int fd) {
    // iterate all players
    for(int i = 0; i < players.size(); i++) {
        Player *player = players[i];

        // if one of players has the file descriptor
        if(player->fd == fd) {
            return player;
        }
    }

    return nullptr;
}

void ServerManager::handlePlayAgain(int fd, vector<string> msgParts) {
    if(msgParts.size() != 1) {
        // if count of message parts is wrong close connection
        cout << "Play again - invalid message parts count" << endl;
        closeConnection(fd, &clientSockets);
        return;
    }

    Game *game = findPlayersGame(fd);
    if(game != nullptr) {
        // if player wants to play new game he cannot be in game
        cout << "Play again - invalid player state - already in game" << endl;
        closeConnection(fd, &clientSockets);
        return;
    }

    // find player with this fd
    Player *player = findPlayerByFd(fd);

    if(player == nullptr) {
        // if player not exists in vector
        cout << "Play again - player is not in players vector" << endl;
        closeConnection(fd, &clientSockets);
        return;
    }

    cout << "Play again - player found" << endl;
    // if player is first in room
    if(games.empty() || games.back()->gameState != Game::GameState::FIRST_CONNECTED) {
        cout << "Play again - player first in game" << endl;
        // create new game
        Game *game = new Game();
        games.push_back(game);
        // add player
        game->addFirstPlayer(player);
        send(fd, SendUtils::playAgainOk(true));
    }
    else {
        cout << "Connect - player second in game" << endl;
        // add player to last room
        Game *game = games.back();
        game->addSecondPlayer(player);
        send(fd, SendUtils::playAgainOk(false));

        if(!game->white->online) {
            // if opponent is offline, send it
            send(fd, SendUtils::opponentOffline());
        }

        // start game
        game->sendGameToPlayers();
        game->gameState = Game::GameState::IN_GAME;
    }
}

void ServerManager::deletePlayersDescriptor(int fd) {
    for(int i = 0; i < players.size(); i++) {
        Player *player = players[i];

        // if one of players has the file descriptor
        if(player->fd == fd) {
            cout << "Client terminated - player marked as terminated and socket was made empty" << endl;
            player->fd = Constants::EMPTY_FD;
            player->terminated = true;
        }
    }
}

void ServerManager::handlePong(int fd, vector <string> msgParts) {
    if(msgParts.size() != 1) {
        // if count of message parts is wrong close connection
        cout << "Pong - invalid message parts count" << endl;
        closeConnection(fd, &clientSockets);
        return;
    }

    Player *player = findPlayerByFd(fd);

    if(player == nullptr) {
        // if player does not exits no need to execute more actions
        return;
    }

    player->lastPongTimestamp = chrono::high_resolution_clock::now();

    if(!player->online) {
        // just when player was offline
        player->online = true;
        cout << "Pong - player online again" << endl;

        // find player's game to notify other player
        Game *game = findPlayersGameInGame(fd);

        if(game != nullptr) {
            // get opponent
            Player *opponent = game->white;
            if(opponent->fd == fd) {
                opponent = game->black;
            }

            // send message that opponent is online again
            cout << "Pong - player online again - opponent notified" << endl;
            send(opponent->fd, SendUtils::opponentOnline());
            send(fd, opponent->online ? SendUtils::opponentOnline() : SendUtils::opponentOffline());
            game->sendGameToPlayers();
        }
    }
}

void ServerManager::checkPlayersOnline() {
    // for all players
    for(int i = 0; i < players.size(); i++) {
        Player *player = players[i];

        // measure distinction from last pong
        auto now = chrono::high_resolution_clock::now();
        auto distinctionMillis = chrono::duration_cast<chrono::milliseconds>(now - player->lastPongTimestamp);

        // just online players
        if(player->online) {
            // if distinction exceeded max allowed value
            if(distinctionMillis.count() > Constants::MAX_PONG_DELAY) {
                player->online = false;
                cout << "Ping - player offline" << endl;

                // find player's game to notify other player
                Game *game = findPlayersGameInGame(player->fd);

                if(game != nullptr) {
                    // get opponent
                    Player *opponent = game->white;
                    if(opponent->fd == player->fd) {
                        opponent = game->black;
                    }

                    // send message that opponent is offline
                    cout << "Ping - player offline - opponent notified" << endl;
                    send(opponent->fd, SendUtils::opponentOffline());
                }
            }
        }
        else {
            // if player is offline, check if he did not exceed the max left time
            if(distinctionMillis.count() > Constants::MAX_LEFT_TIME) {
                // player is offline for too long
                cout << "Ping - player offline for too long" << endl;

                // find player's game
                Game *game = findPlayersGame(player->fd);
                if(game == nullptr) {
                    cout << "Ping - player not in game, deleted from players" << endl;
                    players.erase(players.begin() + findPlayerIdx(player));
                    delete player;
                    i--;
                }
                else {
                    // for white player
                    if(game->white->fd == player->fd) {
                        cout << "Ping - player white leaves game - offline too long" << endl;

                        // setup game and send it to players
                        if(game->black != nullptr) {
                            cout << "Ping - game end - player black is the winner" << endl;
                            game->winner = game->black;
                            game->playing = nullptr;
                            game->gameState = Game::GameState::FINISHED;
                            game->sendGameToPlayers(false, true);
                        }
                    }
                    else {
                        // for black player
                        cout << "Ping - player black" << endl;

                        // setup game and send it to players
                        if(game->white != nullptr) {
                            cout << "Ping - game end - player white is the winner" << endl;
                            game->winner = game->white;
                            game->playing = nullptr;
                            game->gameState = Game::GameState::FINISHED;
                            game->sendGameToPlayers(true, false);
                        }
                    }

                    cout << "Ping - deleting player" << endl;
                    // remove player from vector
                    players.erase(players.begin() + findPlayerIdx(player));
                    delete player;
                    i--;

                    cout << "Ping - deleting game" << endl;
                    // remove game from vector
                    games.erase(games.begin() + findGameIdx(game));
                    delete game;
                }
            }
        }
    }
}

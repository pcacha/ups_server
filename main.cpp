#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "Constants.h"
#include "ServerManager.h"
#include "main.h"
#include <regex>

using namespace std;

// client sockets
fd_set clientSockets;
// server manager
ServerManager serverManager(clientSockets);

/*
 * Binds server socket to port and starts listening on it
 */
void setupServer(int &serverSocket, struct sockaddr_in *myAddr, int port) {
    // setup socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // setup port
    memset(myAddr, 0, sizeof(struct sockaddr_in));
    myAddr->sin_family = AF_INET;
    myAddr->sin_port = htons(port);
    myAddr->sin_addr.s_addr = INADDR_ANY;

    // bind socket to port
    int return_value = bind(serverSocket, (struct sockaddr *) myAddr, sizeof(struct sockaddr_in));
    if (return_value == 0)
        cout << "Bind - OK" << endl;
    else {
        cout << "Bind - error" << endl;
        exit(EXIT_FAILURE);
    }

    // start listening
    return_value = listen(serverSocket, 1000);
    if (return_value == 0){
        cout << "Listen - OK" << endl;
    } else {
        cout << "Bind - error" << endl;
        exit(EXIT_FAILURE);
    }
}

void closeConnection(int fd, fd_set *clientSockets) {
    // delete from games and players and notify other players if needed
    serverManager.disconnectPlayer(fd);
    // socket error
    // close connection and delete appropriate file descriptor
    close(fd);
    FD_CLR(fd, clientSockets);
    cout << "Disconnected - closing socket - fd: " << fd << endl;
}

void closeConnectionWithoutRemovingPlayer(int fd, fd_set *clientSockets) {
    // delete closed fd
    serverManager.deletePlayersDescriptor(fd);
    // socket error
    // close connection and delete appropriate file descriptor
    close(fd);
    FD_CLR(fd, clientSockets);
    cout << "Client terminated - closing socket - fd: " << fd << endl;
}

void send(int fd, string msg) {
    if(FD_ISSET(fd, &clientSockets)) {
        // sends message to client
        send(fd, msg.c_str(), msg.length(), 0);
        cout << "Send - fd: " << fd << " msg: " + msg << endl;
    }
    else {
        // cannot send message
        cout << "Send Failed - can not send message (missing fd) to fd: " << fd << " msg: " + msg << endl;
    }
}

/*
 * Entry point
 * Server starts listening on port
 */
int main() {

    // binding to port and start listening
    int serverSocket;
    struct sockaddr_in myAddr;
    setupServer(serverSocket, &myAddr, 9999);

    // deleting set of descriptors and adding server socket
    FD_ZERO(&clientSockets);
    FD_SET(serverSocket, &clientSockets);

    // neverending loop
    while(true) {
        fd_set tests = clientSockets;
        // select descriptors that was changed
        int returnValue = select(FD_SETSIZE, &tests, nullptr, nullptr, nullptr);

        if (returnValue < 0) {
            cout << "Select - error" << endl;
            exit(EXIT_FAILURE);
        }

        // except stdin, stdout, stderr
        for(int fd = 3; fd < FD_SETSIZE; fd++) {

            // if it server or client socket that was changed
            if(FD_ISSET(fd, &tests)) {

                // if it is server socket, accept new connection
                if(fd == serverSocket) {
                    struct sockaddr_in peerAddr{};
                    auto addrLength = sizeof(sockaddr);
                    int clientSocket = accept(serverSocket, (struct sockaddr *) &peerAddr, (socklen_t *) &addrLength);
                    FD_SET(clientSocket, &clientSockets);
                    cout << "Connect - new client connected - fd: " << clientSocket << endl;
                }
                else {
                    // for client socket, accept data
                    // bytes ready to read
                    int bytesAvailable;
                    ioctl(fd, FIONREAD, &bytesAvailable);

                    // there is something to read
                    /*
                    if(bytesAvailable > 0) {
                        char dataChars[bytesAvailable];
                        read(fd, &dataChars, bytesAvailable);
                        string dataString = dataChars;
                        cout << "Socket - data on socket: " + dataString << endl;

                        while(dataString.length() > 0) {
                            size_t endIdx = dataString.find(Constants::MSG_END[0]);

                            if (endIdx == string::npos) {
                                closeConnection(fd, &clientSockets);
                                break;
                            }

                            string msg = dataString.substr(0, endIdx + 1);
                            if(endIdx + 1 == dataString.length()) {
                                dataString = "";
                            }
                            else {
                                dataString = dataString.substr(endIdx + 1);
                            }

                            size_t lastStartIdx = msg.find_last_of(Constants::MSG_START[0]);

                            if(lastStartIdx != 0) {
                                closeConnection(fd, &clientSockets);
                                break;
                            }

                            if(msg.length() <= Constants::MAX_MSG_LENGTH && msg.length() >= Constants::MIN_MSG_LENGTH ) {
                                cout << "Received - msg: " + msg << endl;
                                serverManager.handleMessage(fd, msg);
                            }
                            else {
                                closeConnection(fd, &clientSockets);
                                break;
                            }
                        }
                    }
                    */


                    /*
                    if(bytesAvailable > 0 && bytesAvailable <= Constants::MAX_MSG_LENGTH) {
                        char dataChars[bytesAvailable];
                        read(fd, &dataChars, bytesAvailable);
                        string dataString = dataChars;
                        cout << "Socket - data on socket: " + dataString << endl;

                        size_t endIdx = dataString.find(Constants::MSG_END[0]);
                        if (endIdx == string::npos || dataString[0] != Constants::MSG_START[0]) {
                            closeConnection(fd, &clientSockets);
                        }

                        string msg = dataString.substr(0, endIdx + 1);
                        if(msg.length() >= Constants::MIN_MSG_LENGTH) {
                            cout << "Received - msg: " + msg << endl;
                            serverManager.handleMessage(fd, msg);
                        }
                        else {
                            closeConnection(fd, &clientSockets);
                        }
                    }
                    else {
                        closeConnection(fd, &clientSockets);
                    }
                    */


                    // bytes available
                    if(bytesAvailable > 0) {
                        // if data are too long, close connection
                        if(bytesAvailable > Constants::MAX_MSG_BATCH_LENGTH) {
                            cout << "Socket - length of batched data exceeded max length" << endl;
                            closeConnection(fd, &clientSockets);
                        }

                        // read all data
                        char dataChars[bytesAvailable];
                        read(fd, &dataChars, bytesAvailable);
                        string dataString = dataChars;
                        cout << "Socket - data on socket: " + dataString << endl;

                        // divide data based on regex
                        regex reg(Constants::MSG_REGEX);
                        smatch matches;
                        regex_search(dataString, matches, reg);

                        // if data are empty - close connection
                        if(matches.empty()) {
                            cout << "Regex - no message  in data" << endl;
                            closeConnection(fd, &clientSockets);
                        }

                        // foreach message in data
                        for(auto match : matches) {
                            string msg = match;

                            // check message length
                            if (msg.length() >= Constants::MIN_MSG_LENGTH && msg.length() <= Constants::MAX_MSG_LENGTH) {
                                cout << "Received - msg: " << msg << endl;
                                serverManager.handleMessage(fd, msg);
                            }
                            else {
                                cout << "Message - invalid length" << endl;
                                closeConnection(fd, &clientSockets);
                            }
                        }
                    }
                    else {
                        cout << "Socket - client terminated" << endl;
                        closeConnectionWithoutRemovingPlayer(fd, &clientSockets);
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
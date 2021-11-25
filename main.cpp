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
#include "SendUtils.h"
#include <regex>
#include <thread>
#include <chrono>
#include <fstream>
#include <signal.h>
#include <arpa/inet.h>

using namespace std;

// client sockets
fd_set clientSockets;
// server manager
ServerManager serverManager(clientSockets);
int serverSocket;

// statistical data
int bytesCount = 0;
int messagesCount = 0;
int connectionsCount = 0;
int closedConnectionsCount = 0;
chrono::high_resolution_clock::time_point programStartTime = chrono::high_resolution_clock::now();

int port;
char *ipAddress;

/*
 * Binds server socket to port and starts listening on it
 */
void setupServer(int &serverSocket, struct sockaddr_in *myAddr) {
    // setup socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket < 0) {
        cout << "Bind - error - cannot create socket" << endl;
        exit(EXIT_FAILURE);
    }

    // setup port
    memset(myAddr, 0, sizeof(struct sockaddr_in));
    myAddr->sin_family = AF_INET;
    myAddr->sin_port = htons(port);

    if(myAddr->sin_port <= 0) {
        cout << "Bind - error - bad port" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Bind - port - " << port << endl;

    myAddr->sin_addr.s_addr = inet_addr(ipAddress);
    //myAddr->sin_addr.s_addr = INADDR_ANY;


    if(myAddr->sin_addr.s_addr <= 0 || myAddr->sin_addr.s_addr == 4294967295) {
        cout << "Bind - error - bad ip address" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Bind - ip address - " << ipAddress << endl;

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
    closedConnectionsCount++;
    cout << "Client terminated - closing socket - fd: " << fd << endl;
}

void send(int fd, string msg) {
    if(FD_ISSET(fd, &clientSockets)) {
        // sends message to client
        send(fd, msg.c_str(), msg.length(), 0);
        if(msg.find(Constants::PING) == string::npos) {
            cout << "Send - fd: " << fd << " msg: " + msg << endl;
        }
    }
    else {
        // cannot send message
        cout << "Send Failed - can not send message (missing fd) to fd: " << fd << " msg: " + msg << endl;
    }
}

// neverending ping system
void pingSystem(int param) {
    chrono::milliseconds sleepTime(Constants::PING_INTERVAL);

    // neverending loop
    while(true) {
        // for all file descriptors
        for(int fd = 3; fd < FD_SETSIZE; fd++) {
            // if it is client socket
            if(FD_ISSET(fd, &clientSockets) && fd != serverSocket) {
                // ping client
                send(fd, SendUtils::ping());
            }
        }

        // check if players are online
        serverManager.checkPlayersOnline();

        this_thread::sleep_for(sleepTime);
    }
}

// at the end of program print statistical data
void printStatisticalData(int sig) {
    // measure program uptime
    auto now = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(now - programStartTime);
    int durationSeconds = duration.count();

    // print data
    fstream dataFile(Constants::STAT_DATA_FILE_NAME, ios::out);
    dataFile << "Transported bytes count: " << bytesCount << endl;
    dataFile << "Transported messages count: " << messagesCount << endl;
    dataFile << "Connections count: " << connectionsCount << endl;
    dataFile << "Unexpectedly closed connections count: " << closedConnectionsCount << endl;
    dataFile << "Uptime in seconds: " << durationSeconds << endl;

    // close file
    dataFile.close();
}

/*
 * Entry point
 * Server starts listening on port
 */
int main(int argc, char *argv[]) {
    if(argc != 3 || !stringIsNumber(argv[2])) {
        cout << "Start - wrong arguments count or port is not a number" << endl;
        exit(EXIT_FAILURE);
    }

    ipAddress = argv[1];
    port = stoi(argv[2]);

    if(port < 1024 || port > 65535) {
        cout << "Start - port has bad range" << endl;
        exit(EXIT_FAILURE);
    }

    if(strcmp(ipAddress, "localhost") == 0 || strcmp(ipAddress, "Localhost") == 0 || strcmp(ipAddress, "LOCALHOST") == 0) {
        cout << "Start - localhost transformed to 127.0.0.1" << endl;
        ipAddress = strdup("127.0.0.1");
    }

    // bind metho that  prints statistical data to program termination
    signal(SIGINT, printStatisticalData);

    // binding to port and start listening
    struct sockaddr_in myAddr;
    setupServer(serverSocket, &myAddr);

    // deleting set of descriptors and adding server socket
    FD_ZERO(&clientSockets);
    FD_SET(serverSocket, &clientSockets);

    // start of ping system
    thread pinger(pingSystem, 0);

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
                    connectionsCount++;
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

                    // bytes available
                    if(bytesAvailable > 0) {
                        bytesCount += bytesAvailable;

                        // if data are too long, close connection
                        if(bytesAvailable > Constants::MAX_MSG_BATCH_LENGTH) {
                            cout << "Socket - length of batched data exceeded max length" << endl;
                            closeConnection(fd, &clientSockets);
                            continue;
                        }

                        // read all data
                        char dataChars[bytesAvailable];
                        read(fd, &dataChars, bytesAvailable);
                        string dataString = dataChars;
                        if(dataString.find(Constants::PONG) == string::npos) {
                            cout << "Socket - data on socket: " + dataString << endl;
                        }


                        // divide data based on regex
                        regex reg(Constants::MSG_REGEX);
                        smatch matches;
                        regex_search(dataString, matches, reg);

                        // if data are empty - close connection
                        if(matches.empty()) {
                            cout << "Regex - no message  in data" << endl;
                            closeConnection(fd, &clientSockets);
                            continue;
                        }

                        // foreach message in data
                        for(auto match : matches) {
                            string msg = match;

                            // check message length
                            if (msg.length() >= Constants::MIN_MSG_LENGTH && msg.length() <= Constants::MAX_MSG_LENGTH) {
                                if(msg.find(Constants::PONG) == string::npos) {
                                    cout << "Received - msg: " << msg << endl;
                                }
                                messagesCount++;
                                serverManager.handleMessage(fd, msg);
                            }
                            else {
                                cout << "Message - invalid length" << endl;
                                closeConnection(fd, &clientSockets);
                                continue;
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

// determines if string is number
bool stringIsNumber(string value) {
    // for all digits
    for(int i = 0; i < value.length(); i++) {
        bool validChar = false;

        // check
        for(int j = 0; j < 10; j++) {

            if(value[i] == to_string(j)[0]) {
                validChar = true;
                break;
            }
        }

        if(!validChar) {
            return false;
        }
    }

    return true;
}

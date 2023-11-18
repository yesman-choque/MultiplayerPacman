#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <sstream>
#include <functional>

#include "../interfaces/game.hpp"

void clientConnection(int listenfd);
void waitOponent(int connfd, int listenfd, Session &session);
void inGameMethod(int connfd, istringstream &iss);


void initializeGame(Session &session) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(0);

    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) cerr << "socket creation failed" << endl;

    int b = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (b == -1) cerr << "bind failed" << endl;

    int l = listen(listenfd, 10);
    if (l == -1) cerr << "listen failed" << endl;

    struct sockaddr_in temp_addr;
    socklen_t temp_len = sizeof(temp_addr);
    getsockname(listenfd, (struct sockaddr *)&temp_addr, &temp_len);
    cout << "Server listening on a random port: " << ntohs(temp_addr.sin_port) << endl;

    session.match.port = ntohs(temp_addr.sin_port);
    session.match.hasOpponent = false;

    createMatrix(session);


    thread waitOponentThread(waitOponent, connfd, listenfd, ref(session));
    waitOponentThread.detach();
 
    thread gameLoopThread(gameLoop, ref(session));
    gameLoopThread.detach();
}

void joinGame(Session &session, string ip, string port) {
    struct sockaddr_in servaddr;

    cout << "IP: " << ip << endl;
    cout << "Port: " << port << endl;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(stoi(port));
    inet_pton(AF_INET, ip.data(), &servaddr.sin_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) cerr << "socket creation failed" << endl;

    int c = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (c == -1) cerr << "connect failed" << endl;

    session.match.connfd = sockfd;
    cout << "TCP Connection Open" << endl;

    thread clientConnectionThread(clientConnection, sockfd);
    clientConnectionThread.detach();
}

void waitOponent(int connfd, int listenfd, Session &session) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    for (;;) {
        connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        cout << "TCP Connection Open" << endl;

        session.match.connfd = connfd;
        session.match.hasOpponent = true;

        thread clientConnectionThread(clientConnection, connfd);
        clientConnectionThread.detach();
    }
}

void clientConnection(int connfd) {
    int n;
    char buff[1000];
    memset(buff, 0, 1000);
    while ((n = read(connfd, buff, 1000)) > 0) {
        buff[n] = 0;

        string message(buff);
        
        cout << message << endl;

        istringstream iss(buff);
        
        string type;
        iss >> type;

        if (type == "in-game") inGameMethod(connfd, iss);

        memset(buff, 0, 1000);
    }
}

void inGameMethod(int connfd, istringstream &iss) {
    string method;
    iss >> method;

    if (method == "delay") {
        string message = "in-game delay-ok";
        write(connfd, message.data(), message.size());

    } else if (method == "end-game") {
    
        string message = "in-game end-game-ok";
        write(connfd, message.data(), message.size());

    } else if (method == "end-game-ok") {
        if (close(connfd) < 0) {
            cout << "Error closing connection" << endl;
        } else {
            cout << "Connection closed" << endl; 
        }
    }
}
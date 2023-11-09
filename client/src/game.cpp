#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <thread>

#include "../interfaces/game.hpp"

void clientConnection(int listenfd);

void initializeGame(Session &session, string ip, string port) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, clientAddr;

    cout << "IP: " << ip << endl;
    cout << "Port: " << port << endl;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(stoi(port));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) cerr << "socket creation failed" << endl;

    int b = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (b == -1) cerr << "bind failed" << endl;

    int l = listen(listenfd, 1);
    if (l == -1) cerr << "listen failed" << endl;


    socklen_t clientAddrLen = sizeof(clientAddr);

    for (;;) {
        connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        cout << "TCP Connection Open" << endl;
        session.gameSocket = connfd;

        thread clientConnectionThread(clientConnection, connfd);
        clientConnectionThread.detach();
    }
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

    session.gameSocket = sockfd;
    cout << "TCP Connection Open" << endl;

    thread clientConnectionThread(clientConnection, sockfd);
    clientConnectionThread.detach();
}

void clientConnection(int listenfd) {
    int n;
    char buff[1000];
    memset(buff, 0, 1000);
    while ((n = read(listenfd, buff, 1000)) > 0) {
        buff[n] = 0;

        string message(buff);
        cout << message << endl;

        memset(buff, 0, 1000);
    }
}
#include "../interfaces/commands.hpp"
#include "../interfaces/session.hpp"
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/select.h>

#define MAXLINE 4096

void singup(Session &session);
void login(Session &session);
void startgame(Session &session);
void initializeGame(Session &session, string ip, string port);

void handleRequest(string line, Session &session) {
    istringstream request(line);
    char recvline[MAXLINE];

    string command;
    request >> command;

    if (command == "novo") {
        request >> session.user >> session.password;
        singup(session);

    } else if (command == "entra") {
        request >> session.user >> session.password;
        login(session);

    } else if (command == "inicia") {
        if (!session.isLogged) return;
        startgame(session);

    } else {
        write(session.serverSocket, line.data(), line.size());
    }
}

void singup(Session &session) {
    cout << session.user << "X" << session.password << endl;
    string message = "auth signin " + session.user + " " + session.password;

    char recvline[MAXLINE];
    int n;

    if (session.protocol == "tcp") {
        write(session.serverSocket, message.data(), message.size());
        n = read(session.serverSocket, recvline, MAXLINE);

    } else if (session.protocol == "udp") {
        sendto(session.serverSocket, message.data(), message.size(), 0, (struct sockaddr *)&session.serverAddress, sizeof(session.serverAddress));
        n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);
    }

    recvline[n] = 0;

    string response(recvline);

    if (response == "auth signin-ok") {
        cout << "User has been registered" << endl;
    } else if (response == "auth signin-nok") {
        cout << "User already exists" << endl;
    }
}

void login(Session &session) {
    string message = "auth login " + session.user + " " + session.password;

    char recvline[MAXLINE];
    int n;

    if (session.protocol == "tcp") {
        write(session.serverSocket, message.data(), message.size());
        n = read(session.serverSocket, recvline, MAXLINE);

    } else if (session.protocol == "udp") {
        sendto(session.serverSocket, message.data(), message.size(), 0, (struct sockaddr *)&session.serverAddress, sizeof(session.serverAddress));
        n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);
    }

    recvline[n] = 0;
    string response(recvline);

    if (response == "auth login-ok") {
        cout << "User has logged in" << endl;
        session.isLogged = true;
    } else if (response == "auth login-nok") {
        cout << "Login is incorrect or user is already logged" << endl;
    }
}

void startgame(Session &session) {
    string message = "connection start";

    char recvline[MAXLINE];
    int n;

    if (session.protocol == "tcp") {
        write(session.serverSocket, message.data(), message.size());
        n = read(session.serverSocket, recvline, MAXLINE);

    } else if (session.protocol == "udp") {
        sendto(session.serverSocket, message.data(), message.size(), 0, (struct sockaddr *)&session.serverAddress, sizeof(session.serverAddress));
        n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);
    }

    recvline[n] = 0;
    string response(recvline);

    if (response == "connection start-nok") {
        cout << "Game has not started" << endl;
        return;
    }

    istringstream iss(response);
    string type;
    string method;
    string ip;
    string port;
    iss >> type >> method >> ip >> port;

    cout << "IP: " << ip << endl;
    cout << "Port: " << port << endl;

    initializeGame(session, ip, port);
}

void initializeGame(Session &session, string ip, string port) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, clientAddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(stoi(port));
    inet_pton(AF_INET, ip.data(), &servaddr.sin_addr);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 1);

    socklen_t clientAddrLen = sizeof(clientAddr);

    for (;;) {
        connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        cout << "TCP Connection Open" << endl;
    }
}

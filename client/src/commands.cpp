#include "../interfaces/commands.hpp"
#include "../interfaces/session.hpp"
#include "../interfaces/game.hpp"
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <poll.h>
#include <thread>

#define MAXLINE 4096

void singup(Session &session);
void login(Session &session);
void startgame(Session &session);
void challenge(Session &session, string opponent);

void transmit(Session &session, string message) {
    if (session.protocol == "tcp") {
        write(session.serverSocket, message.data(), message.size());
    } else if (session.protocol == "udp") {
        sendto(session.serverSocket, message.data(), message.size(), 0, (struct sockaddr *)&session.serverAddress, sizeof(session.serverAddress));
    }
}

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


    } else if (command == "desafio") {
        if (!session.isLogged) return;

        string opponent;
        request >> opponent;

        challenge(session, opponent);

    } else if (command == "move") {

        string movement;
        request >> movement;

        cout << session.gameSocket << endl;

        write(session.gameSocket, movement.data(), movement.size());
    } else {
        write(session.serverSocket, line.data(), line.size());

        struct pollfd fds[1];
        fds[0].fd = session.serverSocket;
        fds[0].events = POLLIN;

        int n = poll(fds, 1, 5000);

        if (n == 0) {
            cout << "Server is not responding" << endl;
            return;
        } else {
            cout << "Server responds" << endl;

            char buffer[MAXLINE];
            int n = read(session.serverSocket, buffer, MAXLINE);
            buffer[n] = 0;

            string response(buffer);
            cout << response << endl;
        }
    }
}

void singup(Session &session) {
    cout << session.user << "X" << session.password << endl;
    string message = "auth signin " + session.user + " " + session.password;

    transmit(session, message);

    char recvline[MAXLINE]; int n;
    n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);

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

    transmit(session, message);
    char recvline[MAXLINE]; int n;
    //n = read(session.serverSocket, recvline, MAXLINE);
    n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);

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

    transmit(session, message);
    n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);

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

    thread initializeGameThread(initializeGame, ref(session), ip, port);
    initializeGameThread.detach();
}

void challenge(Session &session, string opponent) {
    string message = "connection challenge " + opponent;

    transmit(session, message);

    char recvline[MAXLINE];
    int n;

    n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);

    recvline[n] = 0;
    string response(recvline);

    if (response == "connection challenge-nok") {
        cout << "Challenge has not been sent" << endl;
        return;
    }

    string type, connection, ip, port;
    istringstream iss(response);
    iss >> type >> connection >> ip >> port;

    thread initializeGameThread(joinGame, ref(session), ip, port);
    initializeGameThread.detach();
}
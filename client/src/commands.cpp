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

#include <chrono>
using namespace chrono;

#define MAXLINE 4096

void singup(Session &session);
void login(Session &session);
void startgame(Session &session);
void challenge(Session &session, string opponent);

void transmit(Session &session, string message) {
    if (session.protocol == "tcp") {
        write(session.serverSocket, message.data(), message.size());
    } else if (session.protocol == "udp") {
        cout << "sou cliente udp" << endl;
        sendto(session.serverSocket, message.data(), message.size(), 0, (struct sockaddr *)&session.serverAddress, sizeof(session.serverAddress));
    }
}

int handleRequest(string line, Session &session) {
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
        if (!session.isLogged) return 0;
        startgame(session);


    } else if (command == "desafio") {
        if (!session.isLogged) return 0;

        string opponent;
        request >> opponent;

        challenge(session, opponent);

    } else if (command == "move") {

        string movement;
        request >> movement;

        write(session.match.connfd, movement.data(), movement.size());

    } else if (command == "sai") {
        if (!session.isLogged) return 0;
        if (session.isPlaying) return 0;

        string message = "connection logout";
        transmit(session, message);

        char recvline[MAXLINE];
        int n;
        n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);
        recvline[n] = 0;

        string response(recvline);
        if (response == "connection logout-ok") {
            cout << "User has logged out" << endl;
            session.isLogged = false;
            return 0;
        } else if (response == "connection logout-nok") {
            cout << "User has not logged out" << endl;
        }

    } else if (command == "tchau") { 
        string message = "auth quit";
        transmit(session, message);

        char recvline[MAXLINE];
        int n;
        n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);
        recvline[n] = 0;

        string response(recvline);
        if (response == "auth quit-ok") {
            cout << "User has quit" << endl;
            return 1;
        } else if (response == "auth quit-nok") {
            cout << "User has not quit" << endl;
        }
    
    } else if (command == "atraso") {
        if (!session.isLogged) return 0;
        if (!session.isPlaying) return 0;

        cout << "atraso testing" << endl;

        auto start = high_resolution_clock::now();

        string message = "in-game delay";
        write(session.match.connfd, message.data(), message.size());
        char recvline[MAXLINE];
        int n;
        n = recvfrom(session.match.connfd, recvline, MAXLINE, 0, NULL, NULL);
        recvline[n] = 0;

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<nanoseconds>(stop - start);

        cout << "Delay: " << duration.count() / (double)1e6 << "ms" << endl;

    } else {
        write(session.serverSocket, line.data(), line.size());

        struct pollfd fds[1];
        fds[0].fd = session.serverSocket;
        fds[0].events = POLLIN;

        int n = poll(fds, 1, 5000);

        if (n == 0) {
            cout << "Server is not responding" << endl;
            return 0;
        } else {
            cout << "Server responds" << endl;

            char buffer[MAXLINE];
            int n = read(session.serverSocket, buffer, MAXLINE);
            buffer[n] = 0;

            string response(buffer);
            cout << response << endl;
        }
    }

    return 0;
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
    again: n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);

    recvline[n] = 0;
    string response(recvline);

    if (response == "auth login-ok") {
        cout << "User has logged in" << endl;
        session.isLogged = true;
    } else if (response == "auth login-nok") {
        cout << "Login is incorrect or user is already logged" << endl;
    } else {
        goto again;
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

    session.isPlaying = true;

    thread initializeGameThread(initializeGame, ref(session));
    initializeGameThread.join();

    message = "connection gamestart " + to_string(session.match.port);
    cout << message << endl;
    transmit(session, message);

    n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);
    recvline[n] = 0;

    string response2(recvline);
    if (response2 == "connection gamestart-nok") {
        cout << "Game has not started" << endl;
        return;
    }
}

void challenge(Session &session, string opponent) {
    string message = "connection challenge " + opponent;

    transmit(session, message);

    char recvline[MAXLINE];
    int n;

    n = recvfrom(session.serverSocket, recvline, MAXLINE, 0, NULL, NULL);

    recvline[n] = 0;
    string response(recvline);

    cout << response << endl;

    if (response == "connection challenge-nok") {
        cout << "Challenge has not been sent" << endl;
        return;
    }

    session.isPlaying = true;

    string type, connection, ip, port;
    istringstream iss(response);
    iss >> type >> connection >> ip >> port;

    thread initializeGameThread(joinGame, ref(session), ip, port);
    initializeGameThread.detach();
}
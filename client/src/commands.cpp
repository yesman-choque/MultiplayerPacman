#include "../interfaces/commands.hpp"

#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>

#include "../interfaces/game.hpp"
#include "../interfaces/session.hpp"
using namespace chrono;

#define MAXLINE 4096

void transmit(Session &session, string message) {
    if (session.protocol == "tcp") {
        send(session.serverSocket, message.data(), message.size(), 0);
    } else if (session.protocol == "udp") {
        sendto(session.serverSocket, message.data(), message.size(), 0,
               (struct sockaddr *)&session.serverAddress,
               sizeof(session.serverAddress));
    }
}

int handleRequest(string line, Session &session) {
    istringstream request(line);

    string command;
    request >> command;

    if (command == "novo") {
        request >> session.user >> session.password;
        string message = "auth signin " + session.user + " " + session.password;
        transmit(session, message);
    }

    else if (command == "entra") {
        request >> session.user >> session.password;
        string message = "auth login " + session.user + " " + session.password;
        transmit(session, message);
    }

    else if (command == "senha") {
        if (!session.isLogged) return 0;

        string oldpassword, newpassword;
        request >> oldpassword >> newpassword;

        string message = "auth passwd " + oldpassword + " " + newpassword;
        transmit(session, message);
    }

    else if (command == "inicia") {
        if (!session.isLogged) return 0;
        string message = "connection start";
        transmit(session, message);
    }

    else if (command == "desafio") {
        if (!session.isLogged) return 0;

        string opponent;
        request >> opponent;

        string message = "connection challenge " + opponent;
        transmit(session, message);
    }

    else if (command == "move") {
        if (!session.isLogged) return 0;
        if (!session.isPlaying) return 0;

        string movement;
        request >> movement;

        if (session.match.isHost) {
            if (session.match.pacman.hasMoved) return 0;

            movePacman(session, movement);

            if (session.match.hasOpponent) {
                string x = to_string(session.match.pacman.x);
                string y = to_string(session.match.pacman.y);

                string message = "in-game move pacman " + x + " " + y;
                transmitP2P(session, message);
            }

            session.match.pacman.hasMoved = true;

        } else {
            if (session.match.remoteGhost.hasMoved) return 0;
            moveGhost(session, movement);

            cout << "remoteGhost has moved" << endl;

            string x = to_string(session.match.remoteGhost.x);
            string y = to_string(session.match.remoteGhost.y);

            string message = "in-game move remoteGhost " + x + " " + y;

            write(session.match.connfd, message.data(), message.size());

            session.match.remoteGhost.hasMoved = true;
        }
    }

    else if (command == "sai") {
        if (!session.isLogged) return 0;
        if (session.isPlaying) return 0;

        string message = "connection logout";
        transmit(session, message);
    }

    else if (command == "tchau") {
        string message = "auth quit";
        transmit(session, message);
        return 1;
    }

    else if (command == "atraso") {
        if (!session.isLogged) return 0;
        if (!session.isPlaying) return 0;
        
        cout << "Last three delays: " << endl;
        for (double delay : session.match.delays) {
            cout << delay << "ms" << endl;
        }
    }

    else if (command == "encerra") {
        if (!session.isLogged) return 0;
        if (!session.isPlaying) return 0;

        string message = "in-game endgame";
        if (session.match.hasOpponent) {
            transmitP2P(session, message);
        }

        session.isPlaying = false;

        transmit(session, message);
    }

    else if (command == "l") {
        if (!session.isLogged) return 0;

        string message = "connection list";
        transmit(session, message);
    }

    else if (command == "lideres") {
        if (!session.isLogged) return 0;

        string message = "connection leaderboard";
        transmit(session, message);
    }

    return 0;
}
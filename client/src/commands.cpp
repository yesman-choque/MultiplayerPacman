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
        send(session.serverSocket, message.data(), message.size(), 0);
    } else if (session.protocol == "udp") {
        cout << "sou cliente udp" << endl;

        sendto(session.serverSocket, message.data(), message.size(), 0, (struct sockaddr *)&session.serverAddress, sizeof(session.serverAddress));
    }
}

string receive(Session &session) {
    vector<char> buff(MAXLINE, 0);
    int n;
    if (session.protocol == "tcp") {
        n = read(session.serverSocket, buff.data(), buff.size());
    } else if (session.protocol == "udp") {
        n = recvfrom(session.serverSocket, buff.data(), buff.size(), 0, NULL, NULL);
    }

    return string(buff.begin(), buff.begin() + n);
}

int handleRequest(string line, Session &session) {
    istringstream request(line);

    string command;
    request >> command;

    if (command == "novo") {
        request >> session.user >> session.password;
        singup(session);
    } 
    
    else if (command == "entra") {
        request >> session.user >> session.password;
        login(session);
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
        startgame(session);
    } 
    
    else if (command == "desafio") {
        if (!session.isLogged) return 0;

        string opponent;
        request >> opponent;

        challenge(session, opponent);
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
                
                int n = write(session.match.connfd, message.data(), message.size());
                
                if (n >= 0) cout << "pacman has moved" << endl;
                else cout << "pacman has not moved" << endl;
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
    
    } 
    
    else if (command == "encerra") {
        if (!session.isLogged) return 0;
        if (!session.isPlaying) return 0;

        string message = "in-game endgame";
        if (session.match.hasOpponent) {
            transmitP2P(session, message);            
        } else {
            close(session.match.listenfd);
            session.isPlaying = false;
        }

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
    
    else {
        // write(session.serverSocket, line.data(), line.size());

        // struct pollfd fds[1];
        // fds[0].fd = session.serverSocket;
        // fds[0].events = POLLIN;

        // int n = poll(fds, 1, 5000);

        // if (n == 0) {
        //     cout << "Server is not responding" << endl;
        //     return 0;
        // } else {
        //     cout << "Server responds" << endl;

        //     char buffer[MAXLINE];
        //     int n = read(session.serverSocket, buffer, MAXLINE);
        //     buffer[n] = 0;

        //     string response(buffer);
        //     cout << response << endl;
        // }
    }

    return 0;
}

void singup(Session &session) {
    cout << session.user << " " << session.password << endl;
    string message = "auth signin " + session.user + " " + session.password;

    transmit(session, message);
}

void login(Session &session) {
    string message = "auth login " + session.user + " " + session.password;

    transmit(session, message);
}

void startgame(Session &session) {
    string message = "connection start";
    transmit(session, message);
}

void challenge(Session &session, string opponent) {
    string message = "connection challenge " + opponent;

    transmit(session, message);
}
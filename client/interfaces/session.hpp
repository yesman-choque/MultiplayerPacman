#pragma once

#include <string>
#include <netinet/in.h>
#include <vector>
#include <list>
#include <chrono>

using namespace std;

struct Pacman {
    int x;
    int y;
    bool hasMoved;
    int pacdots;
};

struct Ghost {
    int x;
    int y;
    bool hasMoved;
};

struct Match {
    uint16_t port;
    int connfd;
    int listenfd;
    struct sockaddr_in socket;
    vector<vector<char>> matrix;

    bool hasOpponent;
    bool isHost;

    chrono::high_resolution_clock::time_point start;
    list<double> delays;

    Pacman pacman;
    Ghost remoteGhost;
    Ghost botGhost;
    list<pair<int, int>> pacdots;

    Match() {
        delays.push_front(0);
        delays.push_front(0);
        delays.push_front(0);
    }
};

struct Session {
    string user;
    string password;

    bool isLogged;
    bool isPlaying;
    bool hasExited;

    string protocol;
    int serverSocket;
    struct sockaddr_in serverAddress;

    Match match;

    Session() {
        this->isLogged = false;
        this->isPlaying = false;
        this->hasExited = false;
    }
};
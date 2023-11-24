#pragma once

#include <string>
#include <netinet/in.h>
#include <vector>

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

    Pacman pacman;
    Ghost remoteGhost;
    Ghost botGhost;
};

struct Session {
    string user;
    string password;

    bool isLogged;
    bool isPlaying;

    string protocol;
    int serverSocket;
    struct sockaddr_in serverAddress;

    Match match;
};
#pragma once

#include <string>
#include <netinet/in.h>
#include <vector>

using namespace std;

struct Pacman {
    int x;
    int y;
    bool hasMoved;
};

struct Ghost {
    int x;
    int y;
};

struct Match {
    uint16_t port;
    int connfd;
    struct sockaddr_in socket;
    vector<vector<char>> matrix;

    bool hasOpponent;

    Pacman pacman;
    Ghost ghost;
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
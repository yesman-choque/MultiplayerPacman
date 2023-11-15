#pragma once

#include <string>
#include <netinet/in.h>

using namespace std;

struct Match {
    uint16_t port;
    int connfd;
    struct sockaddr_in socket;
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
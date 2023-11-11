#pragma once

#include <string>
#include <netinet/in.h>

using namespace std;

typedef struct session Session;

struct session {
    string user;
    string password;
    bool isLogged;
    bool isPlaying;

    int serverSocket;
    struct sockaddr_in serverAddress;
    
    uint16_t gamePort;
    int gameSocket;
    struct sockaddr_in gameAddress;

    string protocol;
};
#pragma once

#include <string>
#include <netinet/in.h>

using namespace std;

struct Heartbeat {
    struct sockaddr_in address;
    int clientfd;
};

struct User {
    string username;
    string password;
    string protocol;
    bool isLogged;
    bool isPlaying;
    bool isHost;
    bool isConnected;
    bool isAlive;

    struct sockaddr_in address;
    int socket;

    Heartbeat heartbeat;

    int gamePort;

    User(int socket, string protocol) {
        this->isLogged = false;
        this->isPlaying = false;
        this->isHost = false;
        this->isConnected = false;
        this->isAlive = true;
        
        this->socket = socket;
        this->protocol = protocol;
    }
};
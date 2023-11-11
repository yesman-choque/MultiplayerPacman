#pragma once

#include <string>
#include <netinet/in.h>

using namespace std;

struct User {
    string username;
    string password;
    string protocol;
    bool isLogged;
    bool isPlaying;
    bool isHost;

    struct sockaddr_in address;
    int socket;
    int gamePort;

    User(int socket, string protocol) {
        this->isLogged = false;
        this->isPlaying = false;
        this->isHost = false;
        this->socket = socket;
        this->protocol = protocol;
    }
};
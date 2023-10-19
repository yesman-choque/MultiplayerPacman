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
    struct sockaddr_in address;
    int socket;

    User(int socket, string protocol) {
        this->isLogged = false;
        this->socket = socket;
        this->protocol = protocol;
    }
};
#include "user.hpp"
#include "commands.hpp"


#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <list>

using namespace std;

void tcpCommunication(int tcpListenfd, list<User> &users);
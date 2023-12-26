#include "user.hpp"
#include "commands.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <list>
#include <sstream>
#include <vector>

void heartbeat(list<User> &users);

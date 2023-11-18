#pragma once

#include <string>
#include "session.hpp"
#include "engine.hpp"

using namespace std;

void initializeGame(Session &session);
void joinGame(Session &session, string ip, string port);
void transmitP2P(Session &session, string message);
string receiveP2P(Session &session);
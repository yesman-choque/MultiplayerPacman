#pragma once

#include <string>
#include "session.hpp"
#include "engine.hpp"

using namespace std;

void initializeGame(Session &session);
void joinGame(Session &session, string ip, string port);
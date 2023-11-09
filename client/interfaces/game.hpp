#pragma once
#include <string>
#include "session.hpp"

using namespace std;

void initializeGame(Session &session, string ip, string port);
void joinGame(Session &session, string ip, string port);
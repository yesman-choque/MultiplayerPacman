#pragma once

#include <string>
#include "./session.hpp"
#include "./engine.hpp"

using namespace std;

#define MAXLINE 4096

int handleRequest(string line, Session &session);
void transmit(Session &session, string message);
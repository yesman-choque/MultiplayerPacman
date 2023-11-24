#pragma once

#include "session.hpp"
#include "game.hpp"
#include "commands.hpp"

#include <thread>
#include <iostream>
#include <chrono>
#include <cstdlib>

using namespace std;

void gameLoop(Session &session);
void gameLoopGhost(Session &session);
void createMatrix(Session &session);
void printMatrix(vector<vector<char>> matrix, Session &session);
void movePacman(Session &session, string movement);
void moveGhost(Session &session, string movement);
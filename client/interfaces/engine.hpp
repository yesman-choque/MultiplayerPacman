#pragma once

#include "session.hpp"
#include "game.hpp"

#include <thread>
#include <iostream>
#include <chrono>

using namespace std;

void gameLoop(Session &session);
void createMatrix(Session &session);
void printMatrix(vector<vector<char>> matrix);
void movePacman(Session &session, string movement);
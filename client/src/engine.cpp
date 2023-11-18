#include "../interfaces/engine.hpp"


void gameLoop(Session &session);
void createMatrix(Session &session);
void movePacman(Session &session, string movement);
void movebot(Session &session);
void printMatrix(vector<vector<char>> matrix, Session &session);

void gameLoop(Session &session) {
    printMatrix(session.match.matrix, session);

    while (true) {
        movebot(session);

        p: if (!session.match.pacman.hasMoved) {
            this_thread::sleep_for(chrono::milliseconds(100));
            goto p;
        }
        session.match.pacman.hasMoved = false;

        printMatrix(session.match.matrix, session);
        this_thread::sleep_for(chrono::seconds(1));

    }
}

void createMatrix(Session &session) {
    std::vector<std::vector<char>> matrix = {
        {'*', '*', '*', '*', '*', '*', '.', '*', '*', '.', '.', '.', ' ', '.', '.', '.', '.', '.', '*', '*', '.', '*', '*', '*', '*', '*', '*'},
        {'*', '*', '*', '*', '*', '*', '.', '*', '*', '.', '*', '*', '*', '*', '*', '*', '*', '.', '*', '*', '.', '*', '*', '*', '*', '*', '*'},
        {'*', '*', '*', '*', '*', '*', '.', '*', '*', '.', '*', '.', '.', ' ', '.', '.', '*', '.', '*', '*', '.', '*', '*', '*', '*', '*', '*'},
        {'.', '.', '.', '.', '.', ' ', '.', '.', '.', '.', '*', '.', '.', ' ', '.', '.', '*', '.', '.', '.', '.', '.', '.', '.', ' ', '.', '.'},
        {'*', '*', '*', '*', '*', '*', '.', '*', '*', '.', '*', '.', '.', ' ', '.', '.', '*', '.', '*', '*', '.', '*', '*', '*', '*', '*', '*'},
    };

    session.match.matrix = matrix;
    session.match.ghost = { 3, 24 };
    session.match.pacman = { 2, 13 };
}

// move pacman
void movePacman(Session &session, string movement) {
    int x = session.match.pacman.x;
    int y = session.match.pacman.y;

    int nrows = session.match.matrix.size();
    int ncols = session.match.matrix[0].size();

    if (movement == "w") {
        int pos = x - 1 < 0 ? nrows - 1 : x - 1;

        if (session.match.matrix[pos][y] != '*') {
            session.match.pacman.x = pos;
        }
    } else if (movement == "s") {
        int pos = (x + 1) % nrows;

        if (session.match.matrix[pos][y] != '*') {
            session.match.pacman.x = pos;
        }
    } else if (movement == "a") {
        int pos = y - 1 < 0 ? ncols - 1 : y - 1;

        if (session.match.matrix[x][pos] != '*') {
            session.match.pacman.y = pos;
        }
    } else if (movement == "d") {
        int pos = (y + 1) % ncols;

        if (session.match.matrix[x][pos] != '*') {
            session.match.pacman.y = pos;
        }
    }
}

// move bot randomly
void movebot(Session &session) {
    int x = session.match.ghost.x;
    int y = session.match.ghost.y;

    bool hasMoved = false;
    int nrows = session.match.matrix.size();
    int ncols = session.match.matrix[0].size();

    p: int direction = rand() % 4;

    if (direction == 0) {
        int pos = y - 1 < 0 ? ncols - 1 : y - 1;

        if (session.match.matrix[x][pos] != '*') {
            session.match.ghost.y = pos;
            hasMoved = true;
        }
    } else if (direction == 1) {
        int pos = (y + 1) % ncols;

        if (session.match.matrix[x][pos] != '*') {
            session.match.ghost.y = pos;
            hasMoved = true;
        }
    } else if (direction == 2) {
        int pos = x - 1 < 0 ? nrows - 1 : x - 1;

        if (session.match.matrix[pos][y] != '*') {
            session.match.ghost.x = pos;
            hasMoved = true;
        }
    } else if (direction == 3) {
        int pos = (x + 1) % nrows;

        if (session.match.matrix[pos][y] != '*') {
            session.match.ghost.x = pos;
            hasMoved = true;
        }
    }

    if (!hasMoved) goto p;
}

void printMatrix(vector<vector<char>> matrix, Session &session) {
    int nrows = matrix.size();
    int ncols = matrix[0].size();

    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++)
            if (session.match.pacman.x == i && session.match.pacman.y == j) {
                cout << "\033[1;33m" << "P" << "\033[0m";
            } else if (session.match.ghost.x == i && session.match.ghost.y == j) {
                cout << "\033[1;31m" << "F" << "\033[0m";
            } else if (matrix[i][j] == '*') {
                cout << "\033[1;34m" << matrix[i][j] << "\033[0m";
            } else {
                cout << matrix[i][j];
            }
        cout << endl;
    }
}

bool hasCollideWithPacman(Session &session) {
    bool equalX = session.match.pacman.x == session.match.ghost.x;
    bool equalY = session.match.pacman.y == session.match.ghost.y;
    if (equalX && equalY) return true;
    else return false;
}

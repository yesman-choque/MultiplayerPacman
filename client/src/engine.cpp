#include "../interfaces/engine.hpp"


void gameLoop(Session &session);
void createMatrix(Session &session);
void movePacman(Session &session, string movement);
void movebot(Session &session);
void printMatrix(vector<vector<char>> matrix, Session &session);
bool hasCollideWithPacman(Session &session);
bool hasCapturePacdot(Session &session);


void gameLoopGhost(Session &session) {
    printMatrix(session.match.matrix, session);

    session.match.botGhost.hasMoved = true;
    session.match.remoteGhost.hasMoved = true;
    session.match.pacman.hasMoved = false;

    while (!session.match.pacman.hasMoved) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    printMatrix(session.match.matrix, session);
    
    while (true) {
        session.match.botGhost.hasMoved = false;

        while (!session.match.botGhost.hasMoved) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        printMatrix(session.match.matrix, session);

        session.match.remoteGhost.hasMoved = false;
        
        while (!session.match.remoteGhost.hasMoved) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        printMatrix(session.match.matrix, session);

        session.match.pacman.hasMoved = false;
        
        while (!session.match.pacman.hasMoved) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        hasCapturePacdot(session);
        printMatrix(session.match.matrix, session);
    }
}

void gameLoop(Session &session) {
    printMatrix(session.match.matrix, session);

    int pacdots = 0;

    while (true) {
        movebot(session);

        this_thread::sleep_for(chrono::seconds(1));
        
        if (session.match.hasOpponent) {
            string x = to_string(session.match.botGhost.x);
            string y = to_string(session.match.botGhost.y);

            string message = "in-game move botGhost " + x + " " + y;
            transmitP2P(session, message);
        }

        printMatrix(session.match.matrix, session);
        

        if (hasCollideWithPacman(session)) {
            cout << "Game Over" << endl;

            string message = "in-game gameover";
            transmit(session, message);
            string response = receive(session);

            if (response == "in-game gameover-ok") {
                cout << "Game Over" << endl;
                session.isPlaying = false;
                printMatrix(session.match.matrix, session);
                break;
            }
        }

        if (session.match.hasOpponent) {
            session.match.remoteGhost.hasMoved = false;

            while (!session.match.remoteGhost.hasMoved) {
                this_thread::sleep_for(chrono::milliseconds(100));
            }

            printMatrix(session.match.matrix, session);
        }

        session.match.pacman.hasMoved = false;

        while (!session.match.pacman.hasMoved) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        if (hasCapturePacdot(session)) {
            pacdots++;
        }

        if (hasCollideWithPacman(session)) {
            cout << "Game Over" << endl;

            string message = "in-game gameover";
            transmit(session, message);
            string response = receive(session);

            if (response == "in-game gameover-ok") {
                cout << "Game Over" << endl;
                session.isPlaying = false;
                printMatrix(session.match.matrix, session);
                break;
            }
        }

        printMatrix(session.match.matrix, session);
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
    session.match.botGhost = { 3, 24 };
    session.match.remoteGhost = {3, 5};
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

void moveGhost(Session &session, string movement) {
    int x = session.match.remoteGhost.x;
    int y = session.match.remoteGhost.y;

    int nrows = session.match.matrix.size();
    int ncols = session.match.matrix[0].size();

    if (movement == "w") {
        int pos = x - 1 < 0 ? nrows - 1 : x - 1;

        if (session.match.matrix[pos][y] != '*') {
            session.match.remoteGhost.x = pos;
        }
    } else if (movement == "s") {
        int pos = (x + 1) % nrows;

        if (session.match.matrix[pos][y] != '*') {
            session.match.remoteGhost.x = pos;
        }
    } else if (movement == "a") {
        int pos = y - 1 < 0 ? ncols - 1 : y - 1;

        if (session.match.matrix[x][pos] != '*') {
            session.match.remoteGhost.y = pos;
        }
    } else if (movement == "d") {
        int pos = (y + 1) % ncols;

        if (session.match.matrix[x][pos] != '*') {
            session.match.remoteGhost.y = pos;
        }
    }
}

// move bot randomly
void movebot(Session &session) {
    int x = session.match.botGhost.x;
    int y = session.match.botGhost.y;

    bool hasMoved = false;
    int nrows = session.match.matrix.size();
    int ncols = session.match.matrix[0].size();

    p: int direction = rand() % 4;

    if (direction == 0) {
        int pos = y - 1 < 0 ? ncols - 1 : y - 1;

        if (session.match.matrix[x][pos] != '*') {
            session.match.botGhost.y = pos;
            hasMoved = true;
        }
    } else if (direction == 1) {
        int pos = (y + 1) % ncols;

        if (session.match.matrix[x][pos] != '*') {
            session.match.botGhost.y = pos;
            hasMoved = true;
        }
    } else if (direction == 2) {
        int pos = x - 1 < 0 ? nrows - 1 : x - 1;

        if (session.match.matrix[pos][y] != '*') {
            session.match.botGhost.x = pos;
            hasMoved = true;
        }
    } else if (direction == 3) {
        int pos = (x + 1) % nrows;

        if (session.match.matrix[pos][y] != '*') {
            session.match.botGhost.x = pos;
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
                cout << "\033[1;33m" << "P" << "\033[0m ";
            } else if (session.match.botGhost.x == i && session.match.botGhost.y == j) {
                cout << "\033[1;31m" << "F" << "\033[0m ";
            } else if (session.match.hasOpponent && session.match.remoteGhost.x == i && session.match.remoteGhost.y == j) {
                cout << "\033[1;35m" << "f" << "\033[0m ";
            } else if (matrix[i][j] == '*') {
                cout << "\033[1;34m" << matrix[i][j] << "\033[0m ";
            } else {
                cout << matrix[i][j] << " ";
            }
        cout << endl;
    }
}

bool hasCollideWithPacman(Session &session) {
    bool equalX = session.match.pacman.x == session.match.botGhost.x;
    bool equalY = session.match.pacman.y == session.match.botGhost.y;
    if (equalX && equalY) return true;
    else return false;
}

bool hasCapturePacdot(Session &session) {
    int x = session.match.pacman.x;
    int y = session.match.pacman.y;

    if (session.match.matrix[x][y] == '.') {
        session.match.matrix[x][y] = ' ';
        return true;
    } else {
        return false;
    }
}

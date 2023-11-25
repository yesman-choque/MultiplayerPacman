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
        if (!session.isPlaying) return;
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    printMatrix(session.match.matrix, session);

    while (session.isPlaying) {
        session.match.botGhost.hasMoved = false;

        while (!session.match.botGhost.hasMoved) {
            if (!session.isPlaying) return;
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        printMatrix(session.match.matrix, session);

        session.match.remoteGhost.hasMoved = false;

        while (!session.match.remoteGhost.hasMoved) {
            if (!session.isPlaying) return;
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        printMatrix(session.match.matrix, session);

        session.match.pacman.hasMoved = false;

        while (!session.match.pacman.hasMoved) {
            if (!session.isPlaying) return;
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        hasCapturePacdot(session);
        printMatrix(session.match.matrix, session);
    }
}

void gameLoop(Session &session) {
    printMatrix(session.match.matrix, session);

    while (session.isPlaying) {
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
            transmitP2P(session, message);

            string response = receive(session);

            if (response == "in-game gameover-ok") {
                cout << "Game Over" << endl;
                session.isPlaying = false;
                // printMatrix(session.match.matrix, session);
                break;
            }
        }

        if (session.match.hasOpponent) {
            session.match.remoteGhost.hasMoved = false;

            while (session.match.hasOpponent &&
                   !session.match.remoteGhost.hasMoved) {
                this_thread::sleep_for(chrono::milliseconds(100));
            }

            printMatrix(session.match.matrix, session);

            if (hasCollideWithPacman(session)) {
                cout << "Game Over" << endl;

                string message = "in-game gameover";
                transmit(session, message);
                transmitP2P(session, message);
                string response = receive(session);

                if (response == "in-game gameover-ok") {
                    cout << "Game Over" << endl;
                    session.isPlaying = false;
                    // printMatrix(session.match.matrix, session);
                    break;
                }
            }
        }

        session.match.pacman.hasMoved = false;

        cout << "Pacman> " << flush;
        while (!session.match.pacman.hasMoved) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        if (hasCapturePacdot(session)) {
            session.match.pacman.pacdots++;
            if (session.match.pacman.pacdots == 50) {
                cout << "You Win" << endl;

                string message = "in-game gameover";
                transmit(session, message);
                transmitP2P(session, message);
                string response = receive(session);

                if (response == "in-game gameover-ok") {
                    cout << "Game Over" << endl;
                    session.isPlaying = false;
                    break;
                }
            }
        }
        
        printMatrix(session.match.matrix, session);

        if (hasCollideWithPacman(session)) {
            cout << "Game Over" << endl;

            string message = "in-game gameover";
            transmit(session, message);
            transmitP2P(session, message);
            string response = receive(session);

            if (response == "in-game gameover-ok") {
                cout << "Game Over" << endl;
                session.isPlaying = false;
                // printMatrix(session.match.matrix, session);
                break;
            }
        }
    }
}

void createMatrix(Session &session) {
    std::vector<std::vector<char>> matrix = {
        {'*', '*', '*', '*', '*', '*', '.', '*', '*', '.', '.', '.', ' ', '.',
         '.', '.', '.', '.', '*', '*', '.', '*', '*', '*', '*', '*', '*'},
        {'*', '*', '*', '*', '*', '*', '.', '*', '*', '.', '*', '*', '*', '*',
         '*', '*', '*', '.', '*', '*', '.', '*', '*', '*', '*', '*', '*'},
        {'*', '*', '*', '*', '*', '*', '.', '*', '*', '.', '*', '.', '.', ' ',
         '.', '.', '*', '.', '*', '*', '.', '*', '*', '*', '*', '*', '*'},
        {'.', '.', '.', '.', '.', ' ', '.', '.', '.', '.', '*', '.', '.', ' ',
         '.', '.', '*', '.', '.', '.', '.', '.', '.', '.', ' ', '.', '.'},
        {'*', '*', '*', '*', '*', '*', '.', '*', '*', '.', '*', '.', '.', ' ',
         '.', '.', '*', '.', '*', '*', '.', '*', '*', '*', '*', '*', '*'},
    };

    session.match.matrix = matrix;
    session.match.botGhost = {3, 24};
    session.match.remoteGhost = {3, 5};
    session.match.pacman = {2, 13};
    session.match.pacman.pacdots = 0;
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

p:
    int direction = rand() % 4;

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
    system("clear");

    if (session.match.isHost)
        cout << "Pacdots: " << session.match.pacman.pacdots << endl;

    int nrows = matrix.size();
    int ncols = matrix[0].size();

    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
            bool isBotHere =
                session.match.botGhost.x == i && session.match.botGhost.y == j;
            bool isGhostHere = session.match.hasOpponent &&
                               session.match.remoteGhost.x == i &&
                               session.match.remoteGhost.y == j;
            bool isPacmanHere =
                session.match.pacman.x == i && session.match.pacman.y == j;

            if (isBotHere && isGhostHere) {
                cout << "\033[1;35m"
                     << "H"
                     << "\033[0m ";
            } else if (isBotHere) {
                cout << "\033[1;31m"
                     << "F"
                     << "\033[0m ";
            } else if (isGhostHere) {
                cout << "\033[1;35m"
                     << "f"
                     << "\033[0m ";
            } else if (isPacmanHere) {
                cout << "\033[1;33m"
                     << "C"
                     << "\033[0m ";
            } else if (matrix[i][j] == '*') {
                cout << "\033[1;34m" << matrix[i][j] << "\033[0m ";
            } else {
                cout << matrix[i][j] << " ";
            }
        }
        cout << endl;
    }
}

bool hasCollideWithPacman(Session &session) {
    bool hasBotCollided = session.match.pacman.x == session.match.botGhost.x &&
                          session.match.pacman.y == session.match.botGhost.y;
    bool hasGhostCollided =
        session.match.hasOpponent &&
        session.match.pacman.x == session.match.remoteGhost.x &&
        session.match.pacman.y == session.match.remoteGhost.y;
    
    if (hasBotCollided || hasGhostCollided) return true;
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

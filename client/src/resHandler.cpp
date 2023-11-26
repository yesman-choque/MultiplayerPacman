#include "../interfaces/resHandler.hpp"

using namespace std;

void handleAuthResponse(istringstream &iss, Session &session);
void handleConnectionResponse(istringstream &iss, Session &session);
void handleGameResponse(istringstream &iss, Session &session);

void handleResponse(string message, Session &session) {
    istringstream iss(message);
    string type;

    iss >> type;

    if (type == "auth") {
        handleAuthResponse(iss, session);
    }
    
    else if (type == "connection") {
        handleConnectionResponse(iss, session);
    }

    else if (type == "security") {
        string method;
        iss >> method;

        if (method == "heartbeat") {
            cout << "received a heartbeat" << endl;

            string message = "security heartbeat-ok";
            transmit(session, message);
        }
    }

    else if (type == "in-game") {
        handleGameResponse(iss, session);
    }
}

void handleGameResponse(istringstream &iss, Session &session) {
    string method;
    iss >> method;
    if (method == "endgame-ok") {
        cout << "Game has ended" << endl;
        session.isPlaying = false;

        string message = "in-game points " + to_string(session.match.pacman.pacdots);
        transmit(session, message);
    }
    
    else if (method == "endgame-nok") {
        cout << "Game has not ended" << endl;
    }

    else if (method == "points-ok") {
        cout << "Points has been sent" << endl;
    }

    else if (method == "points-nok") {
        cout << "Points has not been sent" << endl;
    }
}

void handleAuthResponse(istringstream &iss, Session &session) {
    string method;
    iss >> method;

    if (method == "signin-ok") {
        cout << "Success to signin" << endl;
    }

    else if (method == "signin-nok") {
        cout << "Failed to signin" << endl;
    }

    else if (method == "login-ok") {
        cout << "Success to login" << endl;
        session.isLogged = true;
    }

    else if (method == "login-nok") {
        cout << "Failed to login" << endl;
    }

    else if (method == "passwd-ok") {
        cout << "Password has been changed" << endl;
    }

    else if (method == "passwd-nok") {
        cout << "Failed to change password" << endl;
    }

    else if (method == "quit-ok") {
        cout << "User has quit" << endl;
        session.hasExited = true;
    }

    else if (method == "quit-nok") {
        cout << "User has not quit" << endl;
    }
}

void handleConnectionResponse(istringstream &iss, Session &session) {
    string method;
    iss >> method;

    if (method == "logout-ok") {
        cout << "User has logged out" << endl;
        session.isLogged = false;
    }

    else if (method == "logout-nok") {
        cout << "User has not logged out" << endl;
    }

    else if (method == "list-ok") {
        string numPlayers;
        iss >> numPlayers;

        cout << "Players online: " << numPlayers << endl;

        for (int i = 0; i < stoi(numPlayers); i++) {
            string username;
            iss >> username;
            cout << username << endl;
        }
    }

    else if (method == "leaderboard-ok") {
        string numPlayers;
        iss >> numPlayers;

        cout << "Table of leaders:" << endl;

        for (int i = 0; i < stoi(numPlayers); i++) {
            string username, points;
            iss >> username >> points;
            cout << username << " " << points << endl;
        }
    }


    else if (method == "start-ok") {
        cout << "Game is starting" << endl;

        session.isPlaying = true;

        thread initializeGameThread(initializeGame, ref(session));
        initializeGameThread.join();

        string message = "connection gamestart " + to_string(session.match.port);
        cout << message << endl;
        transmit(session, message);
    }

    else if (method == "start-nok") {
        cout << "Can't start game" << endl;
    }

    else if (method == "gamestart-ok") {
        cout << "Game has started" << endl;
    }

    else if (method == "gamestart-nok") {
        cout << "Game has not started" << endl;
    }

    else if (method == "challenge-ok") {
        session.isPlaying = true;

        string ip, port;
        iss >> ip >> port;

        thread initializeGameThread(joinGame, ref(session), ip, port);
        initializeGameThread.detach();
    }

    else if (method == "challenge-nok") {
        cout << "Challenge has not been sent" << endl;
    }
}
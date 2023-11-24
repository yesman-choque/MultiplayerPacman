#include "../interfaces/commands.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>

void authType(User &user, list<User> &users, istringstream &iss);
void connectionType(User &user, list<User> &users, istringstream &iss);
void passwd(User &user, string oldPassword, string newPassword);

void transmit(User &user, string message) {
    cout << "entrei aqui" << endl;
    cout << user.protocol << endl;
    cout << user.socket << endl;
    cout << ntohs(user.address.sin_port) << endl;
    cout << user.address.sin_addr.s_addr << endl;

    if (user.protocol == "tcp")
        write(user.socket, message.data(), message.size());
    else if (user.protocol == "udp") {
        sendto(user.socket, message.data(), message.size(), 0,
               (struct sockaddr *)&user.address, sizeof(user.address));
    }
}

void handleRequest(char *buff, User &user, list<User> &users) {
    cout << buff << endl;
    cout << ntohs(user.address.sin_port) << endl;

    istringstream iss(buff);
    string type;

    iss >> type;

    if (type == "auth")
        authType(user, users, iss);
    else if (type == "connection") 
        connectionType(user, users, iss);
    else if (type == "security") {
        string method;
        iss >> method;

        if (method == "heartbeat") {
            string port = to_string(8081);
            string message = "security heartbeat-ok " + port;
            transmit(user, message);
        } else {
            cout << "Invalid method" << endl;
        }

    } else if (type == "in-game") {
        if (!user.isPlaying) return;

        string method;
        iss >> method;

        if (method == "gameover") {
            string message = "in-game gameover-ok";
            user.isPlaying = false;
            transmit(user, message);
        } else if (method == "endgame") {
            string message = "in-game endgame-ok";
            user.isPlaying = false;
            transmit(user, message);
        } else {
            cout << "Invalid method" << endl;
        }

    } else {
        cout << "Invalid type" << endl;
        string message = "Invalid type";
        transmit(user, message);
    }
}

void authType(User &user, list<User> &users, istringstream &iss) {
    string method;
    iss >> method;

    if (method == "signin") {
        iss >> user.username >> user.password;
        signin(user);

    } else if (method == "login") {
        cout << "to em login" << endl;

        iss >> user.username >> user.password;

        login(user, users);

    } else if (method == "quit") {
        if (user.isPlaying) {
            string message = "auth quit-nok";
            transmit(user, message);
            return;
        }

        string message = "auth quit-ok";
        transmit(user, message);
        auto it = users.begin();
        while (it != users.end()) {
            bool isIpEqual =
                it->address.sin_addr.s_addr == user.address.sin_addr.s_addr;
            bool isPortEqual = it->address.sin_port == user.address.sin_port;
            if (isIpEqual && isPortEqual) {
                it = users.erase(it);
                break;
            } else {
                it++;
            }
        }
    } else if (method == "passwd") {
        string oldPassword, newPassword;
        iss >> oldPassword >> newPassword;
        passwd(user, oldPassword, newPassword);
    } else {
        cout << "Invalid method" << endl;
    }
}

void connectionType(User &user, list<User> &users, istringstream &iss) {
    if (!user.isLogged) return;

    string method;
    iss >> method;

    if (method == "start") {
        if (user.isPlaying) {
            string message = "connection start-nok";
            transmit(user, message);
            return;
        }

        string message = "connection start-ok";
        user.isPlaying = true;
        user.isHost = true;

        transmit(user, message);

    } else if (method == "gamestart") {
        string port;
        iss >> port;

        user.gamePort = stoi(port);
        string message = "connection gamestart-ok";
        transmit(user, message);

    } else if (method == "challenge") {
        string opponent;
        iss >> opponent;

        bool found = false;
        int i;

        list<User>::iterator it;

        for (it = users.begin(); it != users.end(); it++) {
            if (it->username == opponent && it->isLogged && it->isPlaying &&
                it->isHost) {
                found = true;
                break;
            }
        }

        if (!found) {
            string message = "connection challenge-nok";
            transmit(user, message);
            return;
        }

        string ip = inet_ntoa(it->address.sin_addr);
        string port = to_string(it->gamePort);

        string message = "connection challenge-ok " + ip + " " + port;
        user.isPlaying = true;

        transmit(user, message);

    } else if (method == "logout") {
        if (!user.isLogged || user.isPlaying) {
            string message = "connection logout-nok";
            transmit(user, message);
            return;
        }

        user.isLogged = false;
        user.isPlaying = false;
        user.isHost = false;

        string message = "connection logout-ok";
        transmit(user, message);
    
    } else if (method == "list") {
        if (!user.isLogged) {
            string message = "connection list-nok";
            transmit(user, message);
            return;
        }

        string message = "connection list-ok ";
        string usersList = "";

        int loggedUsers = 0;
        for (User u : users) {
            if (u.isLogged) {
                loggedUsers++;
                usersList += " " + u.username;
            }
        }
        message += to_string(loggedUsers) + usersList;
        transmit(user, message);

    } else {
        cout << "Invalid method" << endl;
    }
}


void passwd(User &user, string oldPassword, string newPassword) {
    ifstream usersFile("users.txt");
    string line;

    bool found = false;
    while (getline(usersFile, line)) {
        istringstream iss(line);
        string userFile, passFile;

        iss >> userFile >> passFile;

        if (userFile == user.username && passFile == oldPassword) {
            cout << "User found" << endl;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "User not found" << endl;
        string message = "auth passwd-nok";
        transmit(user, message);
        return;
    }

    ifstream usersFile2("users.txt");
    ofstream tempFile("temp.txt");

    while (getline(usersFile2, line)) {
        istringstream iss(line);
        string userFile, passFile;

        iss >> userFile >> passFile;

        if (userFile == user.username) {
            tempFile << userFile << " " << newPassword << endl;
        } else {
            tempFile << userFile << " " << passFile << endl;
        }
    }

    usersFile.close();
    usersFile2.close();
    tempFile.close();

    remove("users.txt");
    rename("temp.txt", "users.txt");

    string message = "auth passwd-ok";
    transmit(user, message);
}

void signin(User &user) {
    ifstream usersFile("users.txt");
    string line;

    bool found = false;
    while (getline(usersFile, line)) {
        istringstream iss(line);
        string userFile, passFile;

        iss >> userFile >> passFile;

        if (userFile == user.username) {
            cout << "User already exists" << endl;
            found = true;
            break;
        }
    }

    if (found) {
        string message = "auth signin-nok";
        transmit(user, message);
        return;
    }

    ofstream serverFile;
    serverFile.open("users.txt", ofstream::binary | ofstream::app);
    serverFile << user.username << " " << user.password << endl;
    serverFile.close();

    string message = "auth signin-ok";
    transmit(user, message);
}

void login(User &user, list<User> &users) {
    ifstream usersFile("users.txt");
    string line;

    bool found = false;
    while (getline(usersFile, line)) {
        istringstream iss(line);
        string userFile, passFile;

        iss >> userFile >> passFile;

        if (userFile == user.username && passFile == user.password) {
            cout << "User found" << endl;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "User not found" << endl;
        string message = "auth login-nok";
        transmit(user, message);

    } else {
        bool userAlreadyLogged = false;

        for (User u : users) {
            if (u.username == user.username && u.isLogged) {
                cout << "User already logged" << endl;
                userAlreadyLogged = true;
                break;
            }
        }

        if (userAlreadyLogged) {
            string message = "auth login-nok";
            transmit(user, message);
            return;
        }

        user.isLogged = true;
        string message = "auth login-ok";

        transmit(user, message);
    }
}

#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

#include "../interfaces/commands.hpp"

void transmit(User &user, string message) {
    if (user.protocol == "tcp")
        write(user.socket, message.data(), message.size());
    else if (user.protocol == "udp")
        sendto(user.socket, message.data(), message.size(), 0, (struct sockaddr *)&user.address, sizeof(user.address));
}

void handleRequest(char *buff, User &user, vector<User> &users) {
    istringstream iss(buff);
    string type;

    iss >> type;

    cout << type << endl;

    if (type == "auth") {

        string method;
        iss >> method;
        cout << method << endl;

        if (method == "signin") {
            iss >> user.username >> user.password;
            signin(user);

        } else if (method == "login") {

            iss >> user.username >> user.password;
            login(user, users);

        } else {
            cout << "Invalid method" << endl;
        }

    } else if (type == "connection") {
        if (!user.isLogged) return;

        cout << "connection type" << endl;
        string method;
        iss >> method;

        if (method == "start") {
            if (user.isPlaying) {
                string message = "connection start-nok";
                transmit(user, message);
                return;
            }

            cout << "IP: " << inet_ntoa(user.address.sin_addr) << endl;
            cout << "Port: " << ntohs(user.address.sin_port) << endl;

            string IP = inet_ntoa(user.address.sin_addr);
            string port = to_string(44888);

            string message = "connection start-ok " + IP + " " + port;
            user.isPlaying = true;
            user.isHost = true;
            transmit(user, message);
        
        } else if (method == "challenge") { 
        
            string opponent;
            iss >> opponent;

            bool found = false;
            int i;
            for (i = 0; i < users.size(); i++) {
                if (users[i].username == opponent && users[i].isLogged && users[i].isPlaying && users[i].isHost) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                string message = "connection challenge-nok";
                transmit(user, message);
                return;
            }

            string ip = inet_ntoa(users[i].address.sin_addr);
            string port = to_string(44888);

            string message = "connection challenge-ok " + ip + " " + port;
            user.isPlaying = true;

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

void login(User &user, vector<User> &users) {
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

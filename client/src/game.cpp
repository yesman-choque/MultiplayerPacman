#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <sstream>
#include <functional>

#include "../interfaces/game.hpp"

void clientConnection(Session &session);
void waitOpponent(int connfd, int listenfd, Session &session);
void inGameMethod(Session &session, istringstream &iss);
void receiveGameData(Session &session);

void transmitP2P(Session &session, string message) {
    write(session.match.connfd, message.data(), message.size());
}

string receiveP2P(Session &session) {
    vector<char> buff(1000, 0);
    int n = read(session.match.connfd, buff.data(), buff.size());
    string message(buff.begin(), buff.begin() + n);

    return message;
}

void initializeGame(Session &session) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(0);

    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) cerr << "socket creation failed" << endl;

    int b = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (b == -1) cerr << "bind failed" << endl;

    int l = listen(listenfd, 10);
    if (l == -1) cerr << "listen failed" << endl;

    struct sockaddr_in temp_addr;
    socklen_t temp_len = sizeof(temp_addr);
    getsockname(listenfd, (struct sockaddr *)&temp_addr, &temp_len);
    cout << "Server listening on a random port: " << ntohs(temp_addr.sin_port) << endl;

    session.match.port = ntohs(temp_addr.sin_port);
    session.match.hasOpponent = false;
    session.match.isHost = true;
    session.match.listenfd = listenfd;

    createMatrix(session);

    thread waitOpponentThread(waitOpponent, connfd, listenfd, ref(session));
    waitOpponentThread.detach();
 
    thread gameLoopThread(gameLoop, ref(session));
    gameLoopThread.detach();
}

void joinGame(Session &session, string ip, string port) {
    struct sockaddr_in servaddr;

    cout << "IP: " << ip << endl;
    cout << "Port: " << port << endl;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(stoi(port));
    inet_pton(AF_INET, ip.data(), &servaddr.sin_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) cerr << "socket creation failed" << endl;

    int c = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (c == -1) cerr << "connect failed" << endl;

    session.match.connfd = sockfd;
    session.match.hasOpponent = true;
    session.match.isHost = false;

    cout << "TCP Connection Open" << endl;

    createMatrix(session);
    receiveGameData(session);

    thread gameLoopThread(gameLoopGhost, ref(session));
    gameLoopThread.detach();

    thread clientConnectionThread(clientConnection, ref(session));
    clientConnectionThread.detach();
}

void waitOpponent(int connfd, int listenfd, Session &session) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (session.isPlaying) {
        connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        cout << "TCP Connection Open" << endl;

        session.match.connfd = connfd;
        session.match.hasOpponent = true;

        thread clientConnectionThread(clientConnection, ref(session));
        clientConnectionThread.detach();
    }
}

void clientConnection(Session &session) {
    int connfd = session.match.connfd;
    int n;
    char buff[1000];
    memset(buff, 0, 1000);

    while ((n = read(connfd, buff, 1000)) > 0) {
        buff[n] = 0;

        string message(buff);
        
        //cout << message << endl;

        istringstream iss(buff);
        
        string type;
        iss >> type;

        if (type == "in-game") inGameMethod(session, iss);

        memset(buff, 0, 1000);
    }

    cout << "Connection closed" << endl;
    session.match.hasOpponent = false;
}

void inGameMethod(Session &session, istringstream &iss) {
    string method;
    iss >> method;
    int connfd = session.match.connfd;

    if (method == "delay") {
        string message = "in-game delay-ok";
        write(connfd, message.data(), message.size());

    } else if (method == "end-game") {
    
        string message = "in-game end-game-ok";
        write(connfd, message.data(), message.size());

    } else if (method == "end-game-ok") {
        if (close(connfd) < 0) {
            cout << "Error closing connection" << endl;
        } else {
            cout << "Connection closed" << endl; 
            session.isPlaying = false;
        }
    } else if (method == "data") {
        string arg;
        iss >> arg;

        if (arg == "pacman") {
            string message = "in-game data pacman-ok";
            message += " " + to_string(session.match.pacman.x);
            message += " " + to_string(session.match.pacman.y);
            transmitP2P(session, message);
            
        } else if (arg == "botGhost") {
            string message = "in-game data botGhost-ok";
            message += " " + to_string(session.match.botGhost.x);
            message += " " + to_string(session.match.botGhost.y);
            transmitP2P(session, message);
        } else if (arg == "remoteGhost") {
            string message = "in-game data remoteGhost-ok";
            message += " " + to_string(session.match.remoteGhost.x);
            message += " " + to_string(session.match.remoteGhost.y);
            transmitP2P(session, message);
        }
    } else if (method == "move") {
        string arg;
        iss >> arg;

        if (arg == "pacman") {
            string x, y;
            iss >> x >> y;

            session.match.pacman.x = stoi(x);
            session.match.pacman.y = stoi(y);


            string message = "in-game move pacman-ok";
            transmitP2P(session, message);

            session.match.pacman.hasMoved = true;
        } else if (arg == "botGhost") {
            string x, y;
            iss >> x >> y;

            session.match.botGhost.x = stoi(x);
            session.match.botGhost.y = stoi(y);

            string message = "in-game move botGhost-ok";
            transmitP2P(session, message);
            session.match.botGhost.hasMoved = true;
        } else if (arg == "remoteGhost") {
            string x, y;
            iss >> x >> y;

            session.match.remoteGhost.x = stoi(x);
            session.match.remoteGhost.y = stoi(y);


            string message = "in-game move remoteGhost-ok";
            transmitP2P(session, message);
            
            session.match.remoteGhost.hasMoved = true;
        }
    } else if (method == "gameover") {
        string message = "in-game gameover-ok";
        session.isPlaying = false;
        transmitP2P(session, message);
        close(connfd);
    } else if (method == "gameover-ok") {
        if (close(connfd) < 0) {
            cout << "Error closing connection" << endl;
        } else {
            cout << "Connection closed" << endl; 
            session.isPlaying = false;
        }
    }
}


void receiveGameData(Session &session) {
    string message = "in-game data pacman";
    transmitP2P(session, message);

    string response = receiveP2P(session);
    istringstream iss(response);

    string type, method, arg, x, y;
    iss >> type >> method >> arg >> x >> y;

    cout << "x: " << x << endl;
    cout << "y: " << y << endl;

    session.match.pacman.x = stoi(x);
    session.match.pacman.y = stoi(y);

    message = "in-game data botGhost";
    transmitP2P(session, message);
    response = receiveP2P(session);
    istringstream iss2(response);
    iss2 >> type >> method >> arg >> x >> y;

    session.match.botGhost.x = stoi(x);
    session.match.botGhost.y = stoi(y);

    message = "in-game data remoteGhost";
    transmitP2P(session, message);
    response = receiveP2P(session);
    istringstream iss3(response);
    iss3 >> type >> method >> arg >> x >> y;

    session.match.remoteGhost.x = stoi(x);
    session.match.remoteGhost.y = stoi(y);
}
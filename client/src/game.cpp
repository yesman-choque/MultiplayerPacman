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

void transmitP2P(Session &session, string message) {
    write(session.match.connfd, message.data(), message.size());
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
    
    string message = "in-game data";
    transmitP2P(session, message);

    thread clientConnectionThread(clientConnection, ref(session));
    clientConnectionThread.detach();

    thread gameLoopThread(gameLoopGhost, ref(session));
    gameLoopThread.detach();
}

void delay(Session &session) {
    while (true) {
        this_thread::sleep_for(chrono::seconds(10));
        if (session.match.hasOpponent) {
            string message = "in-game delay";
            transmitP2P(session, message);
            session.match.start = chrono::high_resolution_clock::now();
        }
    }
}

void waitOpponent(int connfd, int listenfd, Session &session) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (session.isPlaying) {
        connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        session.match.connfd = connfd;
        session.match.hasOpponent = true;

        thread clientConnectionThread(clientConnection, ref(session));
        clientConnectionThread.detach();
    }

    close(session.match.connfd);
    close(session.match.listenfd);
}

void clientConnection(Session &session) {
    int connfd = session.match.connfd;
    int n;
    vector<char> buff(1000, 0);

    thread delayThread(delay, ref(session));
    delayThread.detach();

    while ((n = read(connfd, buff.data(), 1000)) > 0) {
        buff[n] = 0;

        string message(buff.data());
        istringstream iss(message);
        
        string type;
        iss >> type;

        if (type == "in-game") inGameMethod(session, iss);

        buff.assign(buff.size(), 0);
    }
    session.match.hasOpponent = false;
}

void inGameMethod(Session &session, istringstream &iss) {
    string method;
    iss >> method;
    int connfd = session.match.connfd;

    if (method == "delay") {
        string message = "in-game delay-ok";
        transmitP2P(session, message);
    }

    if (method == "delay-ok") {
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(stop - session.match.start);
        double delay = duration.count() / (double)1e6;
        //cout << "Delay: " << delay << "ms" << endl;

        session.match.delays.pop_back();
        session.match.delays.push_front(delay);
    }
    
    else if (method == "endgame") {
        string message = "in-game endgame-ok";
        write(connfd, message.data(), message.size());

        if (!session.match.isHost) {
            session.isPlaying = false;
            close(connfd);

            message = "in-game endgame";
            transmit(session, message);
        }
    }
    
    else if (method == "endgame-ok") {
        if (close(connfd) < 0) {
            cout << "Error closing connection" << endl;
        } else {
            cout << "Connection closed" << endl; 
            session.isPlaying = false;
        }
    }
    
    else if (method == "data") {
        string message = "in-game data-ok";

        // Pacman data
        message += " " + to_string(session.match.pacman.x);
        message += " " + to_string(session.match.pacman.y);

        // Bot Ghost data
        message += " " + to_string(session.match.botGhost.x);
        message += " " + to_string(session.match.botGhost.y);

        // Remote Ghost data
        message += " " + to_string(session.match.remoteGhost.x);
        message += " " + to_string(session.match.remoteGhost.y);
  
        // Pacdots data
        message += " " + to_string(session.match.pacdots.size());
        for (auto [x, y] : session.match.pacdots) {
            message += " " + to_string(x) + " " + to_string(y);
        }

        transmitP2P(session, message);
    }

    else if (method == "data-ok") {
        string x, y;
        iss >> x >> y;

        session.match.pacman.x = stoi(x);
        session.match.pacman.y = stoi(y);

        iss >> x >> y;

        session.match.botGhost.x = stoi(x);
        session.match.botGhost.y = stoi(y);

        iss >> x >> y;

        session.match.remoteGhost.x = stoi(x);
        session.match.remoteGhost.y = stoi(y);

        string size;
        iss >> size;

        int n = stoi(size);

        for (int i = 0; i < n; i++) {
            string x, y;
            iss >> x >> y;

            session.match.pacdots.push_back({stoi(x), stoi(y)});
        }
    }
    
    else if (method == "move") {
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
    }
    
    else if (method == "gameover") {
        string message = "in-game gameover-ok";
        session.isPlaying = false;
        transmitP2P(session, message);
        close(connfd);

        if (!session.match.isHost) {
            message = "in-game endgame";
            transmit(session, message);
        }

        cout << "Game Over" << endl;
        cout << "Pac-Man> " << flush;
    }
    
    else if (method == "gameover-ok") {
        if (close(connfd) < 0) {
            cout << "Error closing connection" << endl;
        } else {
            cout << "Game Over" << endl;
            session.isPlaying = false;
        }
    }
}
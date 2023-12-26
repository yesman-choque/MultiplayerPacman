#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <thread>
#include <tuple>
#include <vector>

#include "../interfaces/commands.hpp"
#include "../interfaces/heartbeat.hpp"
#include "../interfaces/tcp.hpp"
#include "../interfaces/udp.hpp"
#include "../interfaces/user.hpp"

using namespace std;

list<User> users;

tuple<int, int> createSockets(int port);
void adminInput();
void initialReport();

int main(int argc, char *argv[]) {
    // Create TCP and UDP sockets on ports 8080 for the main
    int port = argc > 1 ? atoi(argv[1]) : 8080;

    auto [tcpfd, udpfd] = createSockets(port);

    // Start heartbeat thread
    thread heartbeatThread(heartbeat, ref(users));
    heartbeatThread.detach();

    thread tcpThread(tcpCommunication, tcpfd, ref(users));
    tcpThread.detach();

    thread udpThread(udpCommunication, udpfd, ref(users));
    udpThread.detach();

    initialReport();

    thread adminThread(adminInput);
    adminThread.join();
}

void initialReport() {
    ifstream file("log.txt");
    string line;
    string lastLine = "";

    while (getline(file, line)) {
        lastLine = line;
    }

    if (lastLine == "END OF LOG" || lastLine == "") {
        string level, event, details;
        level = "INFO";
        event = "Server started";
        details = "Server has finished previous execution correctly";
        report(level, event, details);
    }

    else {
        string level, event, details;
        level = "WARNING";
        event = "Server started";
        details = "Server has finished previous execution unexpectedly";
        report(level, event, details);
    }
}

void adminInput() {
    string input;
    while (true) {
        cin >> input;
        if (input == "exit") {
            string level, event, details;
            level = "INFO";
            event = "Server stopped";
            details = "Server has finished execution correctly";
            report(level, event, details);

            ofstream logFile;
            logFile.open("log.txt", ofstream::binary | ofstream::app);
            logFile << "END OF LOG" << endl;
            logFile.close();

            exit(0);
        }
    }
}

tuple<int, int> createSockets(int port) {
    int udpfd, tcpfd;
    struct sockaddr_in serverAddr;

    tcpfd = socket(AF_INET, SOCK_STREAM, 0);
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (tcpfd < 0 || udpfd < 0) {
        cout << "Error creating socket" << endl;
        exit(1);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(tcpfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) {
        cout << "Error binding TCP socket" << endl;
        exit(1);
    }

    if (bind(udpfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) {
        cout << "Error binding UDP socket" << endl;
        exit(1);
    }

    if (listen(tcpfd, 10)) {
        cout << "Error listening TCP socket" << endl;
        exit(1);
    }

    return make_tuple(tcpfd, udpfd);
}

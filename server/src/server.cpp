#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <list>
#include <tuple>

#include "../interfaces/commands.hpp"
#include "../interfaces/user.hpp"
#include "../interfaces/tcp.hpp"
#include "../interfaces/udp.hpp"
#include "../interfaces/heartbeat.hpp"

using namespace std;

list<User> users;

tuple<int, int> createSockets(int port);

int main() {
    // Create TCP and UDP sockets on ports 8080 for the main 
    // communication and 8081 for the heartbeat
    auto [tcpfd, udpfd] = createSockets(8080);
    auto [tcpfd_hb, udpfd_hb] = createSockets(8081);

    // Start heartbeat thread
    thread heartbeatThread(heartbeat, ref(users));
    heartbeatThread.detach();

    thread tcphbThread(tcphb, tcpfd_hb, ref(users));
    tcphbThread.detach();

    thread udpThread_hb(udphb, udpfd_hb, ref(users));
    udpThread_hb.detach();

    thread tcpThread(tcpCommunication, tcpfd, ref(users));
    tcpThread.detach();

    thread udpThread(udpCommunication, udpfd, ref(users));
    udpThread.join();
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

    if (bind(tcpfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr))) {
        cout << "Error binding TCP socket" << endl;
        exit(1);
    }

    if (bind(udpfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr))) {
        cout << "Error binding UDP socket" << endl;
        exit(1);
    }

    if (listen(tcpfd, 5)) {
        cout << "Error listening TCP socket" << endl;
        exit(1);
    }

    return make_tuple(tcpfd, udpfd);
}





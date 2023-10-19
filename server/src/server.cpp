#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <sstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

#include "../interfaces/commands.hpp"
#include "../interfaces/user.hpp"

using namespace std;

void clientConnection(User user);
void tcpCommunication(int tcpListenfd, int connfd);
void udpCommunication(int udpListenfd, struct sockaddr_in clientAddr, socklen_t clientAddrLen);

vector<User> users;

int main() {
    int udpListenfd, tcpListenfd, connfd;

    struct sockaddr_in serverAddr, clientAddr;

    tcpListenfd = socket(AF_INET, SOCK_STREAM, 0);
    udpListenfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8080);

    memset(&clientAddr, 0, sizeof(clientAddr));
    socklen_t clientAddrLen = sizeof(clientAddr);

    bind(tcpListenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    bind(udpListenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    listen(tcpListenfd, 1);

    thread tcpThread(tcpCommunication, tcpListenfd, connfd);
    thread udpThread(udpCommunication, udpListenfd, clientAddr, clientAddrLen);

    tcpThread.join();
    udpThread.join();
}

void clientConnection(User user) {
    int n;
    char buff[1000];
    memset(buff, 0, 1000);
    while ((n = read(user.socket, buff, 1000)) > 0) {
        cout << n << endl;
        handleRequest(buff, user, users);
        memset(buff, 0, 1000);
    }
}

void tcpCommunication(int tcpListenfd, int connfd) {
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    socklen_t clientAddrLen = sizeof(clientAddr);

    for (;;) {
        connfd = accept(tcpListenfd, (struct sockaddr *) &clientAddr, &clientAddrLen);
        cout << "TCP Connection Open" << endl;

        users.push_back(User(connfd, "tcp"));
        users[users.size() - 1].address = clientAddr;

        thread clientThread(clientConnection, users[users.size() - 1]);
        clientThread.detach();
    }
}

void udpCommunication(int udpListenfd, struct sockaddr_in clientAddr, socklen_t clientAddrLen) {
    int n;
    char buff[1000];

    for (;;) {
        n = recvfrom(udpListenfd, buff, 1000, 0, (struct sockaddr *) &clientAddr, (socklen_t *)&clientAddrLen);
        cout << "UDP Connection Open" << endl;

        bool isNewUser = true;
        
        int i;
        for (i = 0; i < users.size(); i++) {
            if (users[i].address.sin_addr.s_addr == clientAddr.sin_addr.s_addr) {
                isNewUser = false;
                break;
            }
        }

        if (isNewUser) {
            users.push_back(User(udpListenfd, "udp"));
            users[users.size() - 1].address = clientAddr;
            handleRequest(buff, users[users.size() - 1], users);
        } else {
            handleRequest(buff, users[i], users);
        }

        memset(buff, 0, 1000);
    }
}
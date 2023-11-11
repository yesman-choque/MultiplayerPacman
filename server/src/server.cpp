#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <sstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <list>

#include "../interfaces/commands.hpp"
#include "../interfaces/user.hpp"

using namespace std;

void clientConnection(User &client);
void tcpCommunication(int tcpListenfd);
void udpCommunication(int udpListenfd);

list<User> users;

int main() {
    int udpListenfd, tcpListenfd;

    struct sockaddr_in serverAddr;

    tcpListenfd = socket(AF_INET, SOCK_STREAM, 0);
    udpListenfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8080);

    bind(tcpListenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    bind(udpListenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    listen(tcpListenfd, 1);

    thread tcpThread(tcpCommunication, tcpListenfd);
    thread udpThread(udpCommunication, udpListenfd);

    tcpThread.join();
    udpThread.join();
}

void clientConnection(User &client) {
    cout << "entrei na função" << endl;

    int n;
    char buff[1000];
    memset(buff, 0, 1000);
    while ((n = read(client.socket, buff, 1000)) > 0) {
        cout << n << endl;
        buff[n] = 0;
        handleRequest(buff, client, users);
        memset(buff, 0, 1000);
    }
}

void tcpCommunication(int tcpListenfd) {
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    socklen_t clientAddrLen = sizeof(clientAddr);

    for (;;) {
        int connfd = accept(tcpListenfd, (struct sockaddr *) &clientAddr, &clientAddrLen);
        cout << "TCP Connection Open" << endl;

        users.push_back(User(connfd, "tcp"));
        users.back().address = clientAddr;

        thread clientThread(clientConnection, ref(users.back()));
        clientThread.detach();
    }
}

void udpClientConnection(int udpListenfd, char* buff, struct sockaddr_in clientAddr, socklen_t clientAddrLen) {
    cout << "UDP Connection Open" << endl;
    bool isNewUser = true;

    cout << clientAddr.sin_addr.s_addr << endl;
    cout << ntohs(clientAddr.sin_port) << endl;

    
    list<User>::iterator it;
    for (it = users.begin(); it != users.end(); it++) {
        bool isIpEqual = it->address.sin_addr.s_addr == clientAddr.sin_addr.s_addr;
        bool isPortEqual = it->address.sin_port == clientAddr.sin_port;
        if (isIpEqual && isPortEqual) {
            isNewUser = false;
            break;
        }
    }

    if (isNewUser) {
        users.push_back(User(udpListenfd, "udp"));
        users.back().address = clientAddr;
        handleRequest(buff, users.back(), users);
    } else {
        handleRequest(buff, *it, users);
    }

    memset(buff, 0, 1000);
}

void udpCommunication(int udpListenfd) {
    int n;
    char buff[1000];

    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));
    socklen_t clientAddrLen = sizeof(clientAddr);

    while ((n = recvfrom(udpListenfd, buff, 1000, 0, (struct sockaddr *) &clientAddr, (socklen_t *)&clientAddrLen)) > 0) {
        buff[n] = 0;
        thread udpClientConnectionThread(udpClientConnection, udpListenfd, buff, clientAddr, clientAddrLen);
        udpClientConnectionThread.detach();
    }
}
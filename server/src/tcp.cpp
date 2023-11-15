#include "../interfaces/tcp.hpp"

void clientConnection(User &client, list<User> &users) {
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

void tcpCommunication(int tcpListenfd, list<User> &users) {
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    socklen_t clientAddrLen = sizeof(clientAddr);

    for (;;) {
        int connfd = accept(tcpListenfd, (struct sockaddr *) &clientAddr, &clientAddrLen);
        cout << "TCP Connection Open" << endl;

        users.push_back(User(connfd, "tcp"));
        users.back().address = clientAddr;

        thread clientThread(clientConnection, ref(users.back()), ref(users));
        clientThread.detach();
    }
}
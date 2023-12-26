#include "../interfaces/tcp.hpp"

void clientConnection(User &client, list<User> &users) {
    vector<char> buff(4096, 0);
    int n;
    while ((n = read(client.socket, buff.data(), buff.size())) > 0) {
        handleRequest(buff.data(), client, users);
        buff.assign(buff.size(), 0);
    }
}

void tcpCommunication(int tcpListenfd, list<User> &users) {
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    socklen_t clientAddrLen = sizeof(clientAddr);

    for (;;) {
        int connfd = accept(tcpListenfd, (struct sockaddr *) &clientAddr, &clientAddrLen);
        cout << "TCP Connection Open" << endl;
        
        // Report connection with client ip
        string level = "INFO";
        string event = "User connected to server via TCP";
        string details = "IP: " + string(inet_ntoa(clientAddr.sin_addr)) + ":" + to_string(ntohs(clientAddr.sin_port));

        report(level, event, details);

        users.push_back(User(connfd, "tcp"));
        users.back().address = clientAddr;

        thread clientThread(clientConnection, ref(users.back()), ref(users));
        clientThread.detach();
    }
}
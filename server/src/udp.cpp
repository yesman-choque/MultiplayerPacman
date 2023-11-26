#include "../interfaces/udp.hpp"

void udpClientConnection(int udpListenfd, string message, struct sockaddr_in clientAddr, list<User> &users) {
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

        handleRequest(message.data(), users.back(), users);
    } else {
        handleRequest(message.data(), *it, users);
    }
}

void udpCommunication(int udpListenfd, list<User> &users) {
    int n;
    char buff[1000];

    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));
    socklen_t clientAddrLen = sizeof(clientAddr);

    while ((n = recvfrom(udpListenfd, buff, 1000, 0, (struct sockaddr *) &clientAddr, (socklen_t *)&clientAddrLen)) > 0) {
        cout << "recebi mensagem" << endl;
        
        buff[n] = 0;
        string message(buff);

        thread udpClientConnectionThread(udpClientConnection, udpListenfd, message, clientAddr, ref(users));
        udpClientConnectionThread.detach();

        memset(buff, 0, 1000);
    }
}
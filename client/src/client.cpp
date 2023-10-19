#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>

#include "../interfaces/commands.hpp"

#define MAXLINE 4096

using namespace std;

int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, "Uso: %s <Endereco IP|Nome> <Porta> <Protocolo>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    Session session;
    session.isLogged = false;
    session.serverAddress = servaddr;

    if (strcmp(argv[3], "tcp") == 0) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        session.serverSocket = sockfd;
        session.protocol = "tcp";
        connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        string line;

        while (getline(cin, line) && line != "tchau") {
            handleRequest(line, session);
        }

    } else if (strcmp(argv[3], "udp") == 0) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        session.serverSocket = sockfd;
        session.protocol = "udp";
        string line;

        cout << "gg" << endl;

        while (getline(cin, line) && line != "tchau") {
            handleRequest(line, session);
        }
    }
}

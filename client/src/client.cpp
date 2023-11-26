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
#include <vector>
#include <thread>

#include "../interfaces/commands.hpp"
#include "../interfaces/resHandler.hpp"

#define MAXLINE 4096

using namespace std;

tuple<string, string> getOwnAdress(int sockfd);
void clientInput(Session &session, int sockfd);
void serverInput(Session &session, int sockfd);

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <Endereco IP|Nome> <Porta> <Protocolo>\n", argv[0]);
        exit(1);
    }

    int serverfd;
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    Session session;
    session.serverAddress = servaddr;

    if (strcmp(argv[3], "tcp") == 0) {
        serverfd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverfd < 0) {
            perror("Erro ao criar socket");
            exit(1);
        }
        session.serverSocket = serverfd;
        session.protocol = "tcp";
        if (connect(serverfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("Erro ao conectar com o servidor");
            exit(1);
        }
    } else if (strcmp(argv[3], "udp") == 0) {
        serverfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (serverfd < 0) {
            perror("Erro ao criar socket");
            exit(1);
        }
        session.serverSocket = serverfd;
        session.protocol = "udp";
    }

    thread serverInputThread(serverInput, ref(session), serverfd);
    serverInputThread.detach();

    thread clientInputThread(clientInput, ref(session), serverfd);
    clientInputThread.join();
}

void serverInput(Session &session, int sockfd) {
    vector<char> buff(1000, 0);

    while (recv(sockfd, buff.data(), buff.size(), 0) > 0) {
        string message(buff.data());
        thread handleResponseThread(handleResponse, message, ref(session));
        handleResponseThread.join();
        buff.assign(buff.size(), 0);
    }
}

void clientInput(Session &session, int sockfd) {
    string line;
    while (getline(cin, line)) {
        int status = handleRequest(line, session);
        if (status) {
            close(sockfd);
            return;
        };
    }
}

tuple<string, string> parseAddress(struct sockaddr_in address) {
    char ip_str_result[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN is the maximum length of an IPv4 address string
    inet_ntop(AF_INET, &address, ip_str_result, sizeof(ip_str_result));

    return make_tuple(string(ip_str_result), to_string(ntohs(address.sin_port)));
}

tuple<string, string> getOwnAdress(int sockfd) {
    struct sockaddr_in temp_addr;
    socklen_t temp_len = sizeof(temp_addr);
    getsockname(sockfd, (struct sockaddr *)&temp_addr, &temp_len);

    char ip_str_result[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN is the maximum length of an IPv4 address string
    inet_ntop(AF_INET, &temp_addr, ip_str_result, sizeof(ip_str_result));


    string ip = inet_ntoa(temp_addr.sin_addr);
    string port = to_string(ntohs(temp_addr.sin_port));

    return make_tuple(ip, port);
}

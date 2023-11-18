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

#define MAXLINE 4096

using namespace std;

void clientInput(Session &session, int sockfd);
tuple<string, string> getOwnAdress(int sockfd);
void heartbeat(Session &session);

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
    session.isLogged = false;
    session.isPlaying = false;
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

        thread heartbeatThread(heartbeat, ref(session));
        heartbeatThread.detach();

        thread clientInputThread(clientInput, ref(session), serverfd);
        clientInputThread.join();
    } else if (strcmp(argv[3], "udp") == 0) {
        serverfd = socket(AF_INET, SOCK_DGRAM, 0);
        session.serverSocket = serverfd;
        session.protocol = "udp";
        
        thread clientInputThread(clientInput, ref(session), serverfd);
        clientInputThread.join();
    }
}

void heartbeat(Session &session) {
    int serverfd;
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8081);
    servaddr.sin_addr = session.serverAddress.sin_addr;

    if (session.protocol == "tcp") {
        serverfd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverfd < 0) {
            perror("Erro ao criar socket");
            exit(1);
        }

        if (connect(serverfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
            perror("Erro ao conectar com o servidor");
            exit(1);
        }

        auto [ip, port] = getOwnAdress(session.serverSocket);

        cout << "IP: " << ip << endl;
        cout << "Port: " << port << endl;

        string message = "security heartbeat " + ip + " " + port;

        write(serverfd, message.data(), message.size());

        vector<char> buff(1000, 0);
        int n = read(serverfd, buff.data(), buff.size());
        string response(buff.data());

        if (response == "security heartbeat-ok") {
            cout << "Connection Established" << endl;
        } else {
            cout << "Connection is Not Established" << endl;
            exit(1);
        }
    } else if (session.protocol == "udp") {
        serverfd = socket(AF_INET, SOCK_DGRAM, 0);
    }    

    vector<char> buff(1000, 0);

    while (true) {
        int n = read(serverfd, buff.data(), buff.size());
        if (n < 0) continue;

        string message(buff.data());
        
        if (message == "connection heartbeat") {
            //cout << "received a heartbeat" << endl;

            string heartbeatResponse = "connection heartbeat-ok";

            if (write(serverfd, heartbeatResponse.data(), heartbeatResponse.size()) < 0) {
                cout << "Error sending heartbeat" << endl;
            };
        }
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

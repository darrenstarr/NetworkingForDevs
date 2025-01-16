// client.cpp
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <IP> <Port> <File>" << std::endl;
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = std::atoi(argv[2]);
    const char *filename = argv[3];

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return 1;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    int valread = read(sock, buffer, BUFFER_SIZE);
    buffer[valread] = '\0';
    if (strcmp(buffer, "ready\n") != 0) {
        std::cerr << "Did not receive 'ready' message from server" << std::endl;
        return 1;
    }

    while (file.good()) {
        file.read(buffer, BUFFER_SIZE);
        send(sock, buffer, file.gcount(), 0);
    }

    file.close();
    close(sock);
    std::cout << "File sent successfully" << std::endl;

    return 0;
}

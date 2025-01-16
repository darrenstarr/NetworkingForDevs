// server.cpp
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

#define PORT 9001
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

void setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int server_fd, new_socket, client_socket[MAX_CLIENTS], max_sd, activity, valread, sd;
    struct sockaddr_in address;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    std::ofstream clientFiles[MAX_CLIENTS];
    char filename[64];

    // Initialize all client_socket to 0
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        // Common reasons for failure
        // - Permission denied
        //    * The ports under 1024 are privileged and require root access
        // - Address already in use
        //    * The port is already in use by another process
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    setNonBlocking(server_fd);

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            std::cerr << "select error" << std::endl;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, NULL, NULL)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            setNonBlocking(new_socket);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    snprintf(filename, sizeof(filename), "/tmp/file_%d.txt", i);
                    clientFiles[i].open(filename);
                    send(new_socket, "ready\n", 6, 0);
                    std::cout << "New connection, file created: " << filename << std::endl;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    close(sd);
                    client_socket[i] = 0;
                    clientFiles[i].close();
                    std::cout << "Client disconnected, file closed: " << i << std::endl;
                } else {
                    buffer[valread] = '\0';
                    clientFiles[i] << buffer;
                }
            }
        }
    }

    return 0;
}

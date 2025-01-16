#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

// Main function to join a multicast group using Source-Specific Multicast (SSM)
// and receive data from the group for 100 seconds
// The program takes three arguments:
// - Source IP address
//     The IP address of the source sending the multicast data
// - Multicast IP address
// - Port number
int main(int argc, char* argv[]) {
    if (argc != 4) {
        // Print usage if the number of arguments is incorrect
        std::cerr << "Usage: " << argv[0] << " <source IP> <multicast IP> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* source_ip = argv[1];
    const char* multicast_ip = argv[2];
    int port = std::atoi(argv[3]);

    int sockfd;
    struct sockaddr_in local_addr{};
    struct ip_mreq_source mreq{};
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Bind the socket to any local address and the specified port
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Join the multicast group using Source-Specific Multicast (SSM)
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_sourceaddr.s_addr = inet_addr(source_ip);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return EXIT_FAILURE;
    }

    std::cout << "Joined multicast group " << multicast_ip << " from source " << source_ip << " on port " << port << std::endl;

    // Receive data from the multicast group for 100 seconds
    time_t start_time = time(nullptr);
    while (time(nullptr) - start_time < 100) {
        ssize_t n = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (n < 0) {
            perror("recv");
            break;
        }
        std::cout << "Received data: " << std::string(buffer, n) << std::endl;
    }

    // Leave the multicast group
    if (setsockopt(sockfd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return EXIT_FAILURE;
    }

    std::cout << "Left multicast group " << multicast_ip << std::endl;

    close(sockfd);
    return EXIT_SUCCESS;
}

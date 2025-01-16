// What I asked Copilot

// write a c++ program which creates a new IPv4 UDP socket 
// and sets options to enable expedited forwarding tagging on 
// the IPv4 level and COS 5 on layer-2. It should send some 
// packets to 192.168.1.1 port 9000

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEST_IP "192.168.1.1"
#define DEST_PORT 9000
#define PACKET_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in dest_addr;
    char buffer[PACKET_SIZE];

    // Create a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Set expedited forwarding (DSCP) on IPv4 level
    int dscp = 0x2E; // DSCP value for Expedited Forwarding (EF) PHB
    if (setsockopt(sockfd, IPPROTO_IP, IP_TOS, &dscp, sizeof(dscp)) < 0) {
        perror("setsockopt IP_TOS");
        close(sockfd);
        return 1;
    }

    // Set CoS 5 (VLAN Priority) on layer-2
    int cos = 5 << 5; // CoS value (shifted to correct position for Priority Code Point)
    if (setsockopt(sockfd, SOL_SOCKET, SO_PRIORITY, &cos, sizeof(cos)) < 0) {
        perror("setsockopt SO_PRIORITY");
        close(sockfd);
        return 1;
    }

    // Initialize destination address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(DEST_PORT);
    inet_pton(AF_INET, DEST_IP, &dest_addr.sin_addr);

    // Send some packets
    for (int i = 0; i < 10; ++i) {
        snprintf(buffer, PACKET_SIZE, "Packet number %d", i);
        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("sendto");
            close(sockfd);
            return 1;
        }
        std::cout << "Sent packet " << i << std::endl;
    }

    // Close the socket
    close(sockfd);
    return 0;
}

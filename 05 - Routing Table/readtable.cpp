#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/rtnetlink.h>

// Structure to hold route information
struct RouteInfo {
    std::string destination; // Destination IP address
    std::string gateway;     // Gateway IP address
    std::string interface;   // Network interface name
};

// Function to convert an IP address from integer to string
std::string ipToString(uint32_t ip) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d",
             ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
    return std::string(buffer);
}

// Function to parse routing information from netlink messages
void parseRoutes(const struct nlmsghdr* nlMsg, std::vector<RouteInfo>& routes) {
    // Iterate over all netlink messages
    for (const struct nlmsghdr* msg = nlMsg; NLMSG_OK(msg, msg->nlmsg_len); msg = NLMSG_NEXT(msg, msg->nlmsg_len)) {
        if (nlMsg->nlmsg_type == NLMSG_DONE) break; // End of messages

        struct rtmsg* rtMsg = (struct rtmsg*)NLMSG_DATA(nlMsg); // Routing message
        if (rtMsg->rtm_family != AF_INET || rtMsg->rtm_table != RT_TABLE_MAIN) continue; // Only handle IPv4 main table

        struct rtattr* rtAttr = (struct rtattr*)RTM_RTA(rtMsg); // Route attributes
        int rtLen = RTM_PAYLOAD(nlMsg); // Length of route attributes

        RouteInfo routeInfo;
        // Iterate over route attributes
        for (; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen)) {
            switch (rtAttr->rta_type) {
                case RTA_DST: // Destination IP address
                    routeInfo.destination = ipToString(*((uint32_t*)RTA_DATA(rtAttr)));
                    break;
                case RTA_GATEWAY: // Gateway IP address
                    routeInfo.gateway = ipToString(*((uint32_t*)RTA_DATA(rtAttr)));
                    break;
                case RTA_OIF: { // Output interface index
                    char ifName[IF_NAMESIZE];
                    if_indextoname(*((int*)RTA_DATA(rtAttr)), ifName);
                    routeInfo.interface = std::string(ifName);
                    break;
                }
            }
        }
        routes.push_back(routeInfo); // Add route info to the list
    }
}

int main() {
    // Create a netlink socket
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock < 0) {
        // Most likely reasons for failure are
        // - Permission denied
        //    * Requires root access to create a raw socket
        // - Protocol not supported
        //    * NETLINK_ROUTE is not supported
        perror("socket");
        return -1;
    }

    // Prepare netlink request
    struct {
        struct nlmsghdr nlHdr; // Netlink message header
        struct rtmsg rtMsg;    // Routing message
    } req;

    memset(&req, 0, sizeof(req));
    req.nlHdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Message length
    req.nlHdr.nlmsg_type = RTM_GETROUTE; // Get routing table
    req.nlHdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP; // Request and dump flags
    req.rtMsg.rtm_family = AF_INET; // IPv4 family

    // Send netlink request to kernel
    if (send(sock, &req, req.nlHdr.nlmsg_len, 0) < 0) {
        perror("send");
        close(sock);
        return -1;
    }

    // Buffer to receive netlink response
    std::vector<char> buffer(4096);
    int len = recv(sock, buffer.data(), buffer.size(), 0);
    if (len < 0) {
        perror("recv");
        close(sock);
        return -1;
    }

    // Parse routes from netlink response
    std::vector<RouteInfo> routes;
    parseRoutes((struct nlmsghdr*)buffer.data(), routes);

    // Print parsed routes
    for (const auto& route : routes) {
        std::cout << "Destination: " << route.destination
                  << ", Gateway: " << route.gateway
                  << ", Interface: " << route.interface << std::endl;
    }

    close(sock); // Close netlink socket
    return 0;
}

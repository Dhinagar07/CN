#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_DOMAINS 100

struct DomainIP {
    char domain[256];
    char ip[INET_ADDRSTRLEN];
};

struct DomainIP domain_map[MAX_DOMAINS] = {
    {"example.com", "93.184.216.34"},
    {"openai.com", "104.18.20.125"},
    {"google.com", "172.217.14.206"},
    {"github.com", "140.82.114.4"},
    {"stackoverflow.com", "151.101.1.69"}
};

int domain_count = 5;  // Set the number of predefined domains

const char* find_ip(const char* domain) {
    for (int i = 0; i < domain_count; i++) {
        if (strcmp(domain_map[i].domain, domain) == 0) {
            return domain_map[i].ip;
        }
    }
    return NULL;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    while (1) {
        recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
        buffer[strcspn(buffer, "\n")] = 0;

        const char* ip_address = find_ip(buffer);
        if (ip_address != NULL) {
            sendto(sockfd, ip_address, strlen(ip_address), 0, (struct sockaddr*)&client_addr, addr_len);
        } else {
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_DGRAM;
            if (getaddrinfo(buffer, NULL, &hints, &res) == 0) {
                char ip[INET_ADDRSTRLEN];
                struct sockaddr_in* ipv4 = (struct sockaddr_in*)res->ai_addr;
                inet_ntop(AF_INET, &ipv4->sin_addr, ip, sizeof(ip));
                sendto(sockfd, ip, strlen(ip), 0, (struct sockaddr*)&client_addr, addr_len);
            } else {
                const char* error_msg = "Error: Could not resolve domain.";
                sendto(sockfd, error_msg, strlen(error_msg), 0, (struct sockaddr*)&client_addr, addr_len);
            }
            freeaddrinfo(res);
        }
    }
    close(sockfd);
    return 0;
}

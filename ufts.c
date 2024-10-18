#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 4096
#define END_OF_FILE "END_OF_FILE"

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_addr_len = sizeof(client_addr);
    FILE *received_file;
    ssize_t bytes_received;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_len);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Receiving file: %s\n", buffer);
        received_file = fopen(buffer, "w");
        if (received_file == NULL) {
            perror("Failed to open file for writing");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    } else {
        perror("Failed to receive file name");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_len);

        if (bytes_received <= 0) {
            printf("Error receiving data or client disconnected.\n");
            break;
        }

        if (strncmp(buffer, END_OF_FILE, strlen(END_OF_FILE)) == 0) {
            printf("File transfer complete.\n");
            break;
        }

        fputs(buffer, received_file);
    }

    printf("File received successfully.\n");

    fclose(received_file);
    close(sockfd);

    return 0;
}
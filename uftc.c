#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 4096
#define END_OF_FILE "END_OF_FILE"

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    FILE *file_to_send;
    ssize_t bytes_read;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("Enter file name to send: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    file_to_send = fopen(buffer, "rb");
    if (file_to_send == NULL) {
        perror("Failed to open file");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file_to_send)) > 0) {
        sendto(sockfd, buffer, bytes_read, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    }

    sendto(sockfd, END_OF_FILE, strlen(END_OF_FILE), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("File sent successfully.\n");

    fclose(file_to_send);
    close(sockfd);

    return 0;
}
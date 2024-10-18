#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 3017
#define BUFFER_SIZE 1024

void send_student_request(const char* server_ip, const char* roll_no) {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Prepare HTTP GET request
    snprintf(buffer, sizeof(buffer), "GET /%s HTTP/1.1\r\nHost: localhost\r\n\r\n", roll_no);
    send(client_socket, buffer, strlen(buffer), 0);

    // Receive response
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Server response:\n%s\n", buffer);
    }

    close(client_socket);
}

int main() {
    char roll_no[BUFFER_SIZE];

    printf("Enter roll number: ");
    scanf("%s", roll_no);

    send_student_request("127.0.0.1", roll_no);

    return 0;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_get_request(int client_socket) {
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 52\r\n"
        "\r\n"
        "<html><body><h1>GET request received</h1></body></html>";
    
    send(client_socket, response, strlen(response), 0);
}

void handle_post_request(int client_socket, const char *request_body) {
    printf("POST request data: %s\n", request_body);

    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 53\r\n"
        "\r\n"
        "<html><body><h1>POST request received</h1></body></html>";

    send(client_socket, response, strlen(response), 0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        read(client_socket, buffer, BUFFER_SIZE);
        printf("Received request:\n%s\n", buffer);

        if (strncmp(buffer, "GET", 3) == 0) {
            handle_get_request(client_socket);
        } else if (strncmp(buffer, "POST", 4) == 0) {
            char *body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4;
                handle_post_request(client_socket, body);
            } else {
                handle_post_request(client_socket, "");
            }
        }

        close(client_socket);
        memset(buffer, 0, BUFFER_SIZE);
    }

    return 0;
}

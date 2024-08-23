#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void send_get_request(int sock) {
    const char *request =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: close\r\n"
        "\r\n";

    send(sock, request, strlen(request), 0);
}

void send_post_request(int sock) {
    const char *body = "name=JohnDoe&age=25";
    char request[BUFFER_SIZE];

    snprintf(request, sizeof(request),
             "POST / HTTP/1.1\r\n"
             "Host: localhost\r\n"
             "Content-Type: application/x-www-form-urlencoded\r\n"
             "Content-Length: %lu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s", strlen(body), body);

    send(sock, request, strlen(request), 0);
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    send_get_request(sock);
    read(sock, buffer, BUFFER_SIZE);
    printf("Response to GET request:\n%s\n", buffer);

    memset(buffer, 0, BUFFER_SIZE);
    close(sock);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    send_post_request(sock);
    read(sock, buffer, BUFFER_SIZE);
    printf("Response to POST request:\n%s\n", buffer);

    close(sock);
    return 0;
}

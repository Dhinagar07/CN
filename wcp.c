#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define CACHE_SIZE 10
#define BUFFER_SIZE 1024
#define PORT 3017
#define MAIN_SERVER_PORT 5633
#define CACHE_TTL 30  // TTL in seconds

// Struct for cache entries
typedef struct {
    char roll_no[BUFFER_SIZE];
    char content[BUFFER_SIZE];
    time_t timestamp;  // Time when the content was cached
} CacheEntry;

// Cache array
CacheEntry cache[CACHE_SIZE];
int cache_index = 0;

// Function to check if a student's marks are in the cache
CacheEntry* check_cache(const char* roll_no) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (strcmp(cache[i].roll_no, roll_no) == 0) {
            time_t current_time = time(NULL);
            if (difftime(current_time, cache[i].timestamp) <= CACHE_TTL) {
                return &cache[i];
            } else {
                // Cache expired, invalidate the cache entry
                memset(&cache[i], 0, sizeof(CacheEntry));
                return NULL;
            }
        }
    }
    return NULL;
}

// Function to add a new entry to the cache
void add_to_cache(const char* roll_no, const char* content) {
    strncpy(cache[cache_index].roll_no, roll_no, BUFFER_SIZE);
    strncpy(cache[cache_index].content, content, BUFFER_SIZE);
    cache[cache_index].timestamp = time(NULL);
    cache_index = (cache_index + 1) % CACHE_SIZE;  // Circular buffer
}

// Function to forward the request to the main server and receive the response
char* forward_request_to_main_server(const char* roll_no) {
    int client_socket;
    struct sockaddr_in server_addr;
    static char buffer[BUFFER_SIZE];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(MAIN_SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Send HTTP GET request
    snprintf(buffer, sizeof(buffer), "GET /%s HTTP/1.1\r\nHost: localhost\r\n\r\n", roll_no);
    send(client_socket, buffer, strlen(buffer), 0);

    // Receive response
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
    }

    close(client_socket);
    return buffer;
}

void handle_client_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';

        // Extract roll number from the request
        char roll_no[BUFFER_SIZE];
        sscanf(buffer, "GET /%s", roll_no);

        CacheEntry* cached_entry = check_cache(roll_no);
        if (cached_entry) {
            // Cache Hit
            printf("Cache Hit for Roll No: %s\n", roll_no);

            // Send cached response with last updated time
            char response[BUFFER_SIZE];
            time_t last_updated = cached_entry->timestamp;
            struct tm* time_info = localtime(&last_updated);
            char time_str[BUFFER_SIZE];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

            snprintf(response, sizeof(response), "%s\nCache Status: HIT\nLast updated: %s\n", cached_entry->content, time_str);
            send(client_socket, response, strlen(response), 0);
        } else {
            // Cache Miss
            printf("Cache Miss for Roll No: %s\n", roll_no);

            // Forward request to main server
            char* server_response = forward_request_to_main_server(roll_no);

            // Cache the new response
            add_to_cache(roll_no, server_response);

            // Send the response to the client with cache status
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), "%s\nCache Status: MISS\n", server_response);
            send(client_socket, response, strlen(response), 0);
        }
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) < 0) {
        perror("Listening failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Proxy server listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Connection acceptance failed");
            continue;
        }

        // Handle the client request
        handle_client_request(client_socket);
    }

    close(server_socket);
    return 0;
}


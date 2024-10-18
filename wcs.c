#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define PORT 5633
#define MAX_SEMS 8    // Maximum number of semesters

// Struct for student data
typedef struct {
    char roll_no[BUFFER_SIZE];
    int marks[MAX_SEMS];
    int n_sems;  // Number of semesters
} Student;

// Sample student database
Student student_db[] = {
    {"101", {85, 90, 75, 80, 95, 88, 92, 85}, 8},
    {"102", {78, 82, 88, 91, 79, 84, 90, 87}, 8},
    {"103", {92, 88, 84, 91, 90, 86, 89, 85}, 8},
};

// Function to fetch student marks based on roll number
Student* fetch_student_marks(const char* roll_no) {
    for (int i = 0; i < sizeof(student_db) / sizeof(Student); i++) {
        if (strcmp(student_db[i].roll_no, roll_no) == 0) {
            return &student_db[i];
        }
    }
    return NULL;  // Student not found
}

void handle_client_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';

        // Extract roll number from the request
        char roll_no[BUFFER_SIZE];
        sscanf(buffer, "GET /%s", roll_no);

        Student* student = fetch_student_marks(roll_no);
        if (student) {
            // Prepare the response
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nRoll No: %s\nMarks:", student->roll_no);
            for (int i = 0; i < student->n_sems; i++) {
                char marks_str[BUFFER_SIZE];
                snprintf(marks_str, sizeof(marks_str), " %d", student->marks[i]);
                strcat(response, marks_str);
            }
            strcat(response, "\n");
            send(client_socket, response, strlen(response), 0);
        } else {
            char* not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nStudent not found\n";
            send(client_socket, not_found, strlen(not_found), 0);
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

    printf("Server listening on port %d...\n", PORT);

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


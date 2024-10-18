#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 5017
#define MAX_PACKETS 10

// Simulating random packet loss (probability of loss: 30%)
int isPacketLost() {
    return (rand() % 100) < 30;
}

// Simulating random acknowledgment loss (probability of loss: 30%)
int isAckLost() {
    return (rand() % 100) < 30;
}

// Simulating delay (in seconds)
void introduceDelay() {
    int delayTime = rand() % 3 + 1; // Random delay between 1 and 3 seconds
    sleep(delayTime);
    printf("Network delay of %d seconds introduced.\n", delayTime);
}

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    srand(time(0));

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the network
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is waiting for a connection...\n");

    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    int packet=1;
    while(packet <= MAX_PACKETS) {
        valread = read(new_socket, buffer, 1024);

        // Simulate packet loss
        if (isPacketLost()) {
            printf("Receiver: Packet %d lost in transit.\n", packet);
        send(new_socket, "NAK", strlen("ACK"), 0);
	    continue;  // Wait for retransmission
        }

        printf("Receiver: Packet %d received.\n", packet);

        // Simulate delay (optional)
        introduceDelay();

        send(new_socket, "ACK", strlen("ACK"), 0);
        printf("Receiver: Acknowledgment for packet %d sent.\n", packet);
	packet++;
    }

    close(new_socket);
    return 0;
}

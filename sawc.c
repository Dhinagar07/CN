#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5017
#define MAX_PACKETS 10

int main() {
    struct sockaddr_in serv_addr;
    int sock = 0, valread;
    char buffer[1024] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert address from text to binary format
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    int packet, attempt;
    for (packet = 1; packet <= MAX_PACKETS; packet++) {
        attempt = 0;
        while (attempt < 5) {  // Maximum 5 attempts for retransmission
            attempt++;
            printf("Sender: Sending packet %d (Attempt %d)\n", packet, attempt);

            send(sock, "PACKET", strlen("PACKET"), 0);
            valread = read(sock, buffer, 1024);

            if (strncmp(buffer, "ACK", 3) == 0) {
                printf("Sender: Acknowledgment for packet %d received.\n", packet);
                break;
            } else {
		    if(strncmp(buffer,"NAK",3)==0)
                printf("Sender: No acknowledgment received for packet %d. Retransmitting...\n", packet);
            }
        }

        if (attempt == 5) {
            printf("Sender: Maximum retransmission attempts reached for packet %d. Aborting.\n", packet);
            close(sock);
            return -1;
        }
    }

    printf("All packets sent successfully.\n");
    close(sock);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345


int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[1024];

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    fprintf(stderr, "Creating server socket\n");
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections and handle them
    client_len = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
        perror("Error accepting connection");
        exit(EXIT_FAILURE);
    }

    printf("Connected to client\n");

    // Receive data from client and echo it back
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            perror("Error receiving data from client");
            exit(EXIT_FAILURE);
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        }

        // Echo back the received data to client
        send(client_socket, buffer, bytes_received, 0);
    }

    // Close the sockets
    close(client_socket);
    close(server_socket);

    return 0;
}

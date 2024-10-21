#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define PORT 8002
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_requests>" << std::endl;
        return -1;
    }

    int num_requests = std::stoi(argv[1]);
    if (num_requests <= 0) {
        std::cerr << "Number of requests must be a positive integer." << std::endl;
        return -1;
    }

    int sock = 0;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    const char *message = "Requesting top CPU processes";

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported" << std::endl;
        close(sock);  // Close socket before returning
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close(sock);  // Close socket before returning
        return -1;
    }

    
    for (int i = 0; i < num_requests; i++) {
        // Send the message to the server
        ssize_t bytes_sent = send(sock, message, strlen(message), 0);
        if (bytes_sent < 0) {
            std::cerr << "Error sending message to server" << std::endl;
            break;
        }
        std::cout << "Message sent: " << message << std::endl;

       
        int net_msg_len;
        if (recv(sock, &net_msg_len, sizeof(net_msg_len), 0) <= 0) {
            std::cerr << "Error reading message length from server." << std::endl;
            break;
        }
        net_msg_len = ntohl(net_msg_len);  // Convert from network byte order

        // Receive the actual response
        ssize_t bytes_received = recv(sock, buffer, net_msg_len, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';  // Null-terminate the received string
            std::cout << "Server response: " << buffer << std::endl;
        } else if (bytes_received < 0) {
            std::cerr << "Error reading response from server." << std::endl;
            break;
        } else {
            std::cerr << "Server closed the connection." << std::endl;
            break;
        }
    }

    // Close the socket
    close(sock);
    return 0;
}

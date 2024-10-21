// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *client_thread(void *arg)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        pthread_exit(NULL);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid address/ Address not supported\n");
        pthread_exit(NULL);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection Failed\n");
        pthread_exit(NULL);
    }

    printf("Connected to server\n");

    // Send a request to the server (dummy request, server doesn't need data)
    char *message = "Request for top 2 CPU processes";
    send(sock, message, strlen(message), 0);

    // Receive server response
    read(sock, buffer, BUFFER_SIZE);
    printf("Server response:\n%s\n", buffer);

    // Close the socket
    close(sock);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <number_of_clients>\n", argv[0]);
        return -1;
    }

    int num_clients = atoi(argv[1]);
    pthread_t threads[num_clients];

    // Create multiple client threads
    for (int i = 0; i < num_clients; i++)
    {
        if (pthread_create(&threads[i], NULL, client_thread, NULL) != 0)
        {
            perror("pthread_create failed");
        }
    }

    // Join threads
    for (int i = 0; i < num_clients; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

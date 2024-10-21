#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Process info structure
typedef struct
{
    char name[256];
    int pid;
    long long user_time;
    long long kernel_time;
    long long total_time;
} process_info;

// Function to read /proc/[pid]/stat and retrieve process info
int get_process_info(process_info *top_two)
{
    DIR *dir = opendir("/proc");
    if (!dir)
    {
        perror("opendir failed");
        return -1;
    }

    struct dirent *entry;
    process_info processes[MAX_CLIENTS];
    int process_count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (isdigit(entry->d_name[0]))
        {
            int pid = atoi(entry->d_name);
            char stat_path[256];
            snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

            FILE *stat_file = fopen(stat_path, "r");
            if (stat_file)
            {
                process_info proc;
                proc.pid = pid;
                fscanf(stat_file, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu",
                       &proc.pid, proc.name, &proc.user_time, &proc.kernel_time);
                fclose(stat_file);

                // Calculate total CPU time (user_time + kernel_time)
                proc.total_time = proc.user_time + proc.kernel_time;

                processes[process_count++] = proc;
                if (process_count == MAX_CLIENTS)
                    break; // Handle up to MAX_CLIENTS processes
            }
        }
    }
    closedir(dir);

    // Sort the processes based on total_time to get top two
    for (int i = 0; i < process_count - 1; i++)
    {
        for (int j = i + 1; j < process_count; j++)
        {
            if (processes[j].total_time > processes[i].total_time)
            {
                process_info temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }

    // Return top two processes
    top_two[0] = processes[0];
    top_two[1] = processes[1];

    return 0;
}

// Function to handle client connection
void *handle_client(void *arg)
{
    int new_socket = *(int *)arg;
    free(arg);

    process_info top_two[2];

    // Retrieve the top two CPU-consuming processes
    if (get_process_info(top_two) == -1)
    {
        char *error_msg = "Error reading process information.";
        send(new_socket, error_msg, strlen(error_msg), 0);
    }
    else
    {
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response),
                 "Top two CPU-consuming processes(Time is in ticks):\n"
                 "1. Name: %s, PID: %d, User Time: %llu, Kernel Time: %llu, Total Time: %llu\n"
                 "2. Name: %s, PID: %d, User Time: %llu, Kernel Time: %llu, Total Time: %llu\n",
                 top_two[0].name, top_two[0].pid, top_two[0].user_time, top_two[0].kernel_time, top_two[0].total_time,
                 top_two[1].name, top_two[1].pid, top_two[1].user_time, top_two[1].kernel_time, top_two[1].total_time);

        // Send response to client
        send(new_socket, response, strlen(response), 0);
    }

    // Close the client socket
    close(new_socket);
    pthread_exit(NULL);
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t thread_id;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Prepare server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);

    // Bind socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for client connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        // Accept a client connection
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0)
        {
            perror("accept failed");
            continue;
        }

        printf("Client connected.\n");

        // Create a new thread to handle the client
        int *socket_ptr = malloc(sizeof(int));
        *socket_ptr = new_socket;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)socket_ptr) != 0)
        {
            perror("pthread_create failed");
        }

        // Detach the thread to allow it to clean up after itself
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}


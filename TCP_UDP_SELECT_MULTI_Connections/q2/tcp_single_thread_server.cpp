#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>

#define PORT 8002
#define MAX 1024

struct ProcessInfo {
    int pid;
    std::string name;
    unsigned long utime;
    unsigned long stime;
};

void send_cpu_info(int client_sock) {
    char response[MAX] = {0};
    int msg_len;

    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        snprintf(response, sizeof(response), "Failed to open /proc directory.\n");
        send(client_sock, response, strlen(response), 0);
        return;
    }

    std::vector<ProcessInfo> processes;
    struct dirent *entry;

    while ((entry = readdir(proc_dir)) != NULL) {
        int pid = atoi(entry->d_name);
        if (pid <= 0) {
            continue;  // Skip non-PID entries
        }

        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        FILE *stat_file = fopen(path, "r");
        if (stat_file == NULL) {
            continue;  
        }

        unsigned long utime, stime;
        char comm[256];
        fscanf(stat_file, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*u %lu %lu",
               &pid, comm, &utime, &stime);
        fclose(stat_file);

        processes.push_back({pid, comm, utime, stime});
    }

    closedir(proc_dir);

    // Sort processes by total CPU time (user + kernel)
    std::sort(processes.begin(), processes.end(), [](const ProcessInfo &a, const ProcessInfo &b) {
        return (a.utime + a.stime) > (b.utime + b.stime);
    });

    snprintf(response, sizeof(response), "Top CPU-consuming processes:\n");
    
    for (size_t i = 0; i < std::min(processes.size(), size_t(2)); ++i) {
        char process_info[256];
        snprintf(process_info, sizeof(process_info), "%zu. %s (PID: %d, User Time: %lu ticks, Kernel Time: %lu ticks)\n",
                 i + 1, processes[i].name.substr(0, sizeof(process_info) - 50).c_str(), processes[i].pid, processes[i].utime, processes[i].stime);
        
        strncat(response, process_info, sizeof(response) - strlen(response) - 1);
    }

    msg_len = strlen(response);
    int net_msg_len = htonl(msg_len);  
    send(client_sock, &net_msg_len, sizeof(net_msg_len), 0);
    send(client_sock, response, msg_len, 0);
}



int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed" << std::endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");  
    address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }
    
    std::cout << "Server is now listening for incoming connections on port " << PORT << std::endl;

    

    // Accept the incoming connection
    while (true) {
        if ((client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;  // Move on to the next connection
        }

        while (true) {
            char buffer[MAX] = {0};
            int valread = read(client_sock, buffer, sizeof(buffer));
            if (valread <= 0) {
                std::cout << "Client disconnected." << std::endl;
                close(client_sock);
                break;  // Exit inner loop to accept new clients
            }

            std::cout << "Message received from client: " << buffer << std::endl;

            // Process the request
            send_cpu_info(client_sock);
        }
    }
 

    return 0;
}


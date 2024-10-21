#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <filesystem>
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <vector>
#include <algorithm>


#define MAX 1024


void create_output_folder();
void start_perf_monitoring();
void handle_client(int new_socket, int client_id);
void send_cpu_info(int client_sock);

struct ProcessInfo {
    int pid;
    std::string name;
    unsigned long utime;
    unsigned long stime;
};

void create_output_folder() {
    const std::string folder_name = "multi_output";
    if (!std::filesystem::exists(folder_name)) {
        if (mkdir(folder_name.c_str(), 0777) == -1) {
            std::cerr << "Failed to create directory: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void handle_client(int new_socket, int client_id) {
    char buffer[MAX] = {0};

    // Reading message from the client
    read(new_socket, buffer, MAX);
    std::cout << "Message received from client " << client_id << ": " << buffer << "\n" << std::endl;

    // Starting performance monitoring for all clients
    start_perf_monitoring();

    send_cpu_info(new_socket);

    close(new_socket);
}

void start_perf_monitoring() {
    std::string filename = "multi_output/perf_output.txt";

    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        std::cerr << "Failed to open perf output file" << std::endl;
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == 0) { 
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);

        // Start performance monitoring including CPU clock, context switches, cache misses, page faults, and CPU migrations
        execlp("perf", "perf", "stat", "--verbose", 
                "-e", "cpu-clock,context-switches,cache-misses,page-faults,cpu-migrations", 
                "-p", std::to_string(getpid()).c_str(), nullptr);

        std::cerr << "Failed to start perf" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        close(fd);
    }
}

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
            continue;
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
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client_id = 0;

    create_output_folder();

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;

    
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (true) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::thread t(handle_client, new_socket, client_id++);
        t.detach();
    }

    close(server_fd);
    return 0;
}


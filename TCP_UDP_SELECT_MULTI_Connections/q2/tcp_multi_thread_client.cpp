#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include <cstdlib> 

void client_task() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char *hello = "Hello from client";
    char buffer[1024] = {0};

    std::thread::id this_id = std::this_thread::get_id();

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Thread " << this_id << ": Socket creation error" << std::endl;
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Thread " << this_id << ": Invalid address/Address not supported" << std::endl;
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Thread " << this_id << ": Connection Failed" << std::endl;
        return;
    }

    send(sock, hello, strlen(hello), 0);
    std::cout << "Thread " << this_id << ": Hello message sent" << std::endl;

    // Read the length of the incoming message
    int net_msg_len;
    recv(sock, &net_msg_len, sizeof(net_msg_len), 0);
    net_msg_len = ntohl(net_msg_len);

    recv(sock, buffer, net_msg_len, 0);
    buffer[net_msg_len] = '\0';  // Null terminate the string

    std::cout << "Thread " << this_id << ": Message received: " << buffer << std::endl;

    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_requests>" << std::endl;
        return 1;
    }

    int num_requests = std::atoi(argv[1]);

    if (num_requests <= 0) {
        std::cerr << "Please provide a valid positive number of requests." << std::endl;
        return 1;
    }

    std::vector<std::thread> threads;

    // Create `num_requests` client threads
    for (int i = 0; i < num_requests; ++i) {
        threads.emplace_back(client_task);
    }

    // Joining all the threads
    for (auto &t : threads) {
        t.join();
    }

    return 0;
}



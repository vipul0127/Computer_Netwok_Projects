Here’s the updated version of the README file with relevant symbols:

---

# 🖥 TCP Client-Server Project

## 🌟 Overview
This project demonstrates the implementation of a TCP client-server communication system, supporting both single-threaded and multi-threaded modes for handling multiple client requests concurrently. The server is designed to handle requests from clients seeking information about the top CPU-consuming processes. The client initiates a connection to the server, sends a request, and receives a response with the process information.

## 🛠 Features
1. **Server**: 
   - 🖧 The server creates a TCP socket and listens for client connections.
   - 💬 It handles multiple concurrent client connections using threads for a multi-threaded server model.
   - 🔄 Server continuously listens for new connections while processing requests from connected clients.
   - 📊 For each client, the server identifies the top CPU-consuming processes, including process name, PID, and CPU usage in user and kernel modes.

2. **Client**:
   - 📡 The client initiates a TCP connection to the server and sends a request for the top two CPU-consuming processes.
   - 🧵 The client supports multiple concurrent connection requests, initiated via multiple threads.
   - 🖥 Upon receiving the server's response, the client displays the process information and closes the connection.

3. **Taskset Usage**: 
   - 🖱 Taskset is used to pin both the client and server to specific CPUs, optimizing CPU usage.

## 📁 Project Structure
```
.
├── client.c         # Client code for initiating connections and requesting process information
├── server.c         # Server code for handling client requests and fetching CPU process info
├── Makefile          # Build instructions for compiling the server and client programs
├── README.md         # Project documentation
└── report.pdf        # Performance report (optional)
```

## 🖥 Server Code
1. **Socket Creation**:
   - 🌐 The server creates a TCP socket using `socket()` with IPv4 and TCP protocols.
   - ⚠️ If the socket creation fails, the program prints an error message and exits.

2. **Binding**:
   - 🔒 The server binds the socket to a specific IP address and port using the `bind()` function.

3. **Listening**:
   - 🎧 The server enters passive listening mode with `listen()`, allowing up to 5 pending client connections.

4. **Handling Multiple Clients**:
   - 📨 The server accepts client connections using `accept()`.
   - 🔄 For each new client, the server creates a new thread using `pthread_create()`, passing the client’s socket to the thread for handling.
   - 👂 The server continues to listen for additional connections while concurrently processing clients.

5. **Identifying Top CPU-Consuming Processes**:
   - 📂 The server reads the `/proc` directory to gather information about running processes.
   - 🥇 It identifies the top two CPU-consuming processes and retrieves the process name, PID, and CPU usage in user and kernel modes.

## 💻 Client Code
1. **Socket Creation**:
   - 🌐 The client creates a TCP socket using `socket()` to establish a connection with the server.

2. **Multiple Concurrent Connections**:
   - 🧵 The client supports initiating multiple concurrent client requests using threads, with the number of threads passed as a command-line argument.

3. **Requesting Process Information**:
   - 📩 The client sends a request to the server, asking for the top two CPU-consuming processes.

4. **Displaying Server Response**:
   - 🖥 The client reads the server’s response and displays the process information (process name, PID, CPU usage in user and kernel modes).

5. **Closing the Connection**:
   - 🛑 After receiving and displaying the response, the client closes the connection to the server.

## 🔧 Compilation and Usage
### Compilation
To compile the server and client programs, navigate to the project directory and run:
```bash
make
```

This will create the `server` and `client` executables.

### Running the Server
To run the server, use the following command:
```bash
./server
```

### Running the Client
To run the client, use the following command with an optional argument for the number of concurrent clients:
```bash
./client <num_clients>
```
Where `<num_clients>` is the number of concurrent client requests to be initiated.

Example:
```bash
./client 5
```

This will initiate 5 concurrent client connections.

## 📊 Performance Analysis
The project also includes performance analysis to compare the different server models:
1. **Single-threaded TCP Client-Server**: This mode handles requests sequentially, with minimal system resource usage.
2. **Multi-threaded TCP Client-Server**: This mode handles requests concurrently, leading to higher CPU and memory usage due to managing multiple threads.
3. **TCP Client-Server using Select**: This mode allows for handling multiple connections using a single thread, offering a balance between resource utilization and concurrency.

The performance statistics (such as task clock, CPU utilization, context switches, page faults, etc.) have been captured for each mode of operation to analyze the system’s efficiency.

### 📈 Key Performance Metrics
- **Task Clock**: ⏱ The total time spent executing tasks on the CPU.
- **CPU Utilization**: 🖥 The percentage of time the CPU is actively executing instructions.
- **Context Switches**: 🔄 The number of switches between kernel and user space during program execution.
- **Page Faults**: 📄 The number of times the system accesses data not currently in memory, triggering a page fault.
- **CPU Core Cycles**: 🔢 The number of cycles executed by the CPU core.

The performance analysis shows the differences in resource utilization between the single-threaded, multi-threaded, and select-based server models, allowing us to understand the trade-offs in concurrency and efficiency.

## ✅ Conclusion
This project showcases how a TCP-based client-server system can be implemented with different concurrency models (single-threaded, multi-threaded, and select-based) for handling client requests. The server's ability to process requests concurrently using threads or select enhances its performance for high-load scenarios, while the single-threaded model is more suitable for simpler applications.

---

This updated README includes various symbols and icons to enhance the readability and presentation of the document.

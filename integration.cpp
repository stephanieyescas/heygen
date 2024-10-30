#include <iostream>
#include <thread>
#include <chrono>
#include "server/server.h"
#include "client/client.h"
#include "proxy/proxy.h"

// Function to run the server
void runServer(unsigned short port) {
    char *argv[] = { nullptr, (char*)std::to_string(port).c_str() }; // Convert port to string
    int argc = 2;
    server(argc, argv); // Start the server
}

// Function to run the proxy
void runProxy(unsigned short proxyPort, const std::string& serverIP, unsigned short serverPort) {
    Proxy proxy(proxyPort, serverIP, serverPort);
    proxy.start();
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_port> <proxy_port>" << std::endl;
        return 1;
    }

    unsigned short serverPort = static_cast<unsigned short>(std::atoi(argv[1]));
    unsigned short proxyPort = static_cast<unsigned short>(std::atoi(argv[2]));

    // Start the server in a separate thread
    std::thread serverThread(runServer, serverPort);
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Allow the server to start

    // Start the proxy
    std::thread proxyThread(runProxy, proxyPort, "127.0.0.1", serverPort);
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Allow the proxy to start

    Client client("127.0.0.1", proxyPort);
    int jobID;

    while (true) {
        std::cout << "Enter job ID to check status (or Ctrl+C to exit): ";
        std::cin >> jobID;

        if (jobID == -1) break;

        client.pollStatus(jobID); // Poll for the new job status
    }

    // Wait for the threads to finish
    serverThread.join();
    proxyThread.join();

    return 0;
}

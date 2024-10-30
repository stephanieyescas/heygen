#include "client.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

// Constructor
Client::Client(const std::string& proxyIP, unsigned short proxyPort)
    : serverIP(proxyIP), port(proxyPort) {}

std::string Client::getStatus(int jobID) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Failed to create client socket");
        return "";
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);
    serverAddress.sin_port = htons(port);

    if (connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("Failed to connect to proxy");
        close(clientSocket);
        return "";
    }

    // Send jobID to proxy
    std::string request = std::to_string(jobID);
    send(clientSocket, request.c_str(), request.length(), 0);

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    close(clientSocket);

    if (bytesRead < 0) {
        perror("Failed to read from proxy");
        return "";
    }

    return std::string(buffer);
}

void Client::pollStatus(int jobID, int maxRetries) {
    int retries = 0;
    int waitTime = 1;
    std::string status; 

    while (retries < maxRetries) {
        status = getStatus(jobID);
        std::cout << "Status: " << status << std::endl;
        
        if (status.find("completed") != std::string::npos) {
            break; // Job is completed
        }

        std::this_thread::sleep_for(std::chrono::seconds(waitTime));
        waitTime = std::min(waitTime * 2, 32); // Exponential backoff
        retries++;
    }
}

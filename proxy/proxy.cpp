#include "proxy.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

Proxy::Proxy(unsigned short port, const std::string& serverIP, unsigned short serverPort)
    : port(port), serverIP(serverIP), serverPort(serverPort) {}

void Proxy::start() {
    int proxySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (proxySocket < 0) {
        std::cerr << "Failed to create proxy socket" << std::endl;
        return;
    }

    struct sockaddr_in proxyAddress;
    memset(&proxyAddress, 0, sizeof(proxyAddress));
    proxyAddress.sin_family = AF_INET;
    proxyAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    proxyAddress.sin_port = htons(port);

    if (bind(proxySocket, (struct sockaddr*)&proxyAddress, sizeof(proxyAddress)) < 0) {
        std::cerr << "Failed to bind proxy socket" << std::endl;
        close(proxySocket);
        return;
    }

    if (listen(proxySocket, 128) < 0) {
        std::cerr << "Failed to listen on proxy socket" << std::endl;
        close(proxySocket);
        return;
    }

    std::cout << "Proxy listening on port " << port << "." << std::endl;

    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t size = sizeof(clientAddress);
        int clientSocket = accept(proxySocket, (struct sockaddr*)&clientAddress, &size);
        if (clientSocket < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        std::thread(&Proxy::handleClient, this, clientSocket).detach();
    }
}

void Proxy::handleClient(int clientSocket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead < 0) {
        perror("Failed to read from client");
        close(clientSocket);
        return;
    }

    int jobID = std::stoi(buffer);
    std::string response = forwardRequest(jobID);

    send(clientSocket, response.c_str(), response.length(), 0);
    close(clientSocket);
}

std::string Proxy::forwardRequest(int jobID) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create server socket");
        return "{\"result\": \"error\"}";
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);
    serverAddress.sin_port = htons(serverPort);

    if (connect(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Failed to connect to translation server");
        close(serverSocket);
        return "{\"result\": \"error\"}";
    }

    send(serverSocket, std::to_string(jobID).c_str(), std::to_string(jobID).length(), 0);

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = recv(serverSocket, buffer, sizeof(buffer) - 1, 0);
    close(serverSocket);

    if (bytesRead < 0) {
        perror("Failed to read from translation server");
        return "{\"result\": \"error\"}";
    }

    return std::string(buffer);
}

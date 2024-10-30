#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>

class Client {
    public:
        Client(const std::string& serverIP, unsigned short port);
        std::string getStatus(int jobID);
        void pollStatus(int jobID, int maxRetries = 10);

    private:
        std::string serverIP;
        unsigned short port;
};

#endif
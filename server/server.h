#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstdlib>
#include <climits>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <arpa/inet.h>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <string>

// Represents a translation job
struct job {
    std::string status;
    std::chrono::steady_clock::time_point start;
    int duration;
};

void handleRequest(int client);
std::string createJSON(const std::string& result);
unsigned short extractPort(const char *arg);
int createServerSocket(unsigned short port);
int server(int argc, char *argv[]);

#endif 
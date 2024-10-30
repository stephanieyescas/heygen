#ifndef PROXY_H
#define PROXY_H

#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <netinet/in.h>

class Proxy {
public:
    Proxy(unsigned short port, const std::string& serverIP, unsigned short serverPort);
    void start();
    
private:
    unsigned short port;
    std::string serverIP;
    unsigned short serverPort;
    
    void handleClient(int clientSocket);
    std::string forwardRequest(int jobID);
};

#endif // PROXY_H

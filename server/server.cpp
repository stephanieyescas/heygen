#include "server.h"
#include "../threadpool/thread-pool.h"

using namespace std;

static const int kServerSocketFailure = -1; 
static const int kReuseAddresses = 1;   
static const int kDefaultBacklog = 128; 
static const int kWrongArgumentCount = 1; 
static const int kIllegalPortArgument = 1; 

unordered_map<int, job> jobs;
std::mutex m;

// JSON response construction
string createJSON(const string& result) {
    return "{\"result\": \"" + result + "\"}";
}

void handleRequest(int client) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = recv(client, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead < 0) {
        cerr << "Failed to read from client" << endl;
        close(client);
        return;
    }

    string request(buffer);
    int jobID = stoi(request); 
    cout << "Received request for job ID: " << jobID << endl;

    {
        std::lock_guard<std::mutex> lock(m);
        if (jobs.find(jobID) == jobs.end()) {
            job temp;
            temp.status = "pending";
            temp.start = std::chrono::steady_clock::now();
            temp.duration = rand() % 10 + 5; 
            cout << "Created new job with ID: " << jobID << " (duration: " << temp.duration << " seconds)" << endl;
            jobs[jobID] = temp; 
        } 
    }

    job currentJob = jobs[jobID]; 

    auto now = std::chrono::steady_clock::now();
    if (currentJob.status == "pending" && 
        std::chrono::duration_cast<std::chrono::seconds>(now - currentJob.start).count() >= currentJob.duration) {
        currentJob.status = "completed";
        cout << "Job ID: " << jobID << " has been completed." << endl;
    }

    string response = createJSON(currentJob.status);
    
    if (currentJob.status == "completed") {
        jobs.erase(jobID);
    }

    send(client, response.c_str(), response.length(), 0);
    close(client);
}

unsigned short extractPort(const char *arg) {
    char *end;
    long port = std::strtoul(arg, &end, 10);
    
    if (*end != '\0' || port < 1024 || port > 65535) {
        return USHRT_MAX; 
    }
    return static_cast<unsigned short>(port);
}

int createServerSocket(unsigned short port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) return kServerSocketFailure;
    
    // Enable address reuse
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &kReuseAddresses, sizeof(int)) < 0) {
        cerr << "Failed to set socket options." << endl;
        close(serverSocket);
        return kServerSocketFailure;
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);

    if (::bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == 0 && 
        listen(serverSocket, kDefaultBacklog) == 0) {
        return serverSocket;
    }

    close(serverSocket);
    return kServerSocketFailure;
}

int server(int argc, char *argv[]) {
    if (argc > 2) {
        cerr << "Usage: " << argv[0] << " [<port>]" << endl;
        return kWrongArgumentCount;
    }
    
    srand(static_cast<unsigned int>(time(nullptr)));

    unsigned short port = extractPort(argv[1]);
    if (port == USHRT_MAX) {
        cerr << "Invalid port number specified" << endl;
        return kIllegalPortArgument;
    }

    int serverSocket = createServerSocket(port);
    if (serverSocket == kServerSocketFailure) {
        cerr << "Failed to create server socket" << endl;
        return -1;
    }

    cout << "Server listening on port " << port << "." << endl;
    ThreadPool pool(16);
    while (true) {
        struct sockaddr_in address;
        socklen_t size = sizeof(address);
        bzero(&address, size);
        int client = accept(serverSocket, (struct sockaddr *)&address, &size);
        if (client < 0) {
            cerr << "Failed to accept connection" << endl;
            continue;
        }

        char str[INET_ADDRSTRLEN];
        cout << "Received a connection request from "
             << inet_ntop(AF_INET, &address.sin_addr, str, INET_ADDRSTRLEN)
             << "." << endl;

        pool.schedule([client]() {
            handleRequest(client);
        });
    }

    return 0;
}

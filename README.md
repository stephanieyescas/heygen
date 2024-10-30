# HeyGen

## Overview
This repository contains a simulated video translation server, proxy, and a client library for querying job statuses. The server simulates video translation tasks with random durations, while the client library provides an interface for polling job statuses.

## Installation

### Prerequisites
- C++17 compatible compiler (e.g., g++)


## Integration Test
The `integration.cpp` file runs the server, proxy, and client together in a multi-threaded environment. It prompts the user for job IDs to check their statuses.

1. Build the integration test.

   ```zsh
   g++ -std=c++17 -o test integration.cpp server/server.cpp client/client.cpp threadpool/thread-pool.cpp proxy/proxy.cpp -lpthread

2. Execute the integration test.
   ```zsh
   ./test <server_port> <proxy_port>

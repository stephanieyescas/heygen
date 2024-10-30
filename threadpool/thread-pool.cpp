#include "thread-pool.h"

// Constructor: initializes the thread pool with a specified number of threads
ThreadPool::ThreadPool(size_t numThreads) : availableWorkers(numThreads) {
    dt = std::thread([this]() { dispatcher(); });
    for (size_t workerID = 0; workerID < numThreads; workerID++) {
        wts.emplace_back([this, workerID]() { worker(workerID); }); 
    }
}

// Dispatcher function
void ThreadPool::dispatcher() {
    while (!terminate) {
        std::unique_lock<std::mutex> lg(m);
        done.wait(lg, [this] { return !jobs.empty() || terminate; });

        if (terminate && jobs.empty()) return;

        // Notify one worker if a job is available
        if (!jobs.empty()) {
            done.notify_one();
        }
    }
}

// Schedule a new job to be executed
void ThreadPool::schedule(const std::function<void(void)>& thunk) {
    {
        std::unique_lock<std::mutex> lg(m);
        jobs.push(thunk); // Add job to the queue
    }
    done.notify_one(); // Notify one waiting worker
}

// Worker function
void ThreadPool::worker(size_t workerID) {
    while (true) {
        std::function<void(void)> job;

        {
            std::unique_lock<std::mutex> lg(m);
            done.wait(lg, [this] { return !jobs.empty() || terminate; });

            if (terminate && jobs.empty()) return; // Exit if terminating

            job = std::move(jobs.front());
            jobs.pop(); // Remove job from the queue
        }

        job(); // Execute the job

        {
            std::lock_guard<std::mutex> lg(m);
            availableWorkers++; // Mark the worker as available
        }
    }
}

// Wait for all jobs to finish
void ThreadPool::wait() {
    std::unique_lock<std::mutex> lg(m);
    done.wait(lg, [this] { return jobs.empty() && availableWorkers == wts.size(); });
}

// Destructor: clean up resources
ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lg(m);
        terminate = true; // Set termination flag
    }
    done.notify_all(); // Notify all threads to wake up

    for (std::thread &t : wts) {
        if (t.joinable()) {
            t.join(); // Wait for all worker threads to finish
        }
    }
    if (dt.joinable()) {
        dt.join(); // Wait for dispatcher thread to finish
    }
}

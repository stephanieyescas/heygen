#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>


class ThreadPool {
 public:
   ThreadPool(size_t numThreads);
   void schedule(const std::function<void(void)>& thunk);
   void wait();
   ~ThreadPool();

 private:
   std::thread dt;                  // dispatcher thread handle
   std::vector<std::thread> wts;    // worker thread handles

   std::mutex m;
   std::condition_variable done;

   std::queue<std::function<void(void)> > jobs; 

   bool terminate = false;
   size_t availableWorkers;

   void dispatcher();
   void worker(size_t workerID);

   ThreadPool(const ThreadPool& original) = delete;
   ThreadPool& operator=(const ThreadPool& rhs) = delete;
}; 

#endif
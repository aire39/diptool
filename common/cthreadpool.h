#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "cjthread.h"
#include "cthread.h"

class cthreadpool
{
  public:
    cthreadpool() = delete;
    explicit cthreadpool(size_t n_threads);
    explicit cthreadpool(const size_t && n_threads, const std::string && t_pool_name);
    explicit cthreadpool(size_t n_threads, const std::string & t_pool_name);
    ~cthreadpool();

    void addjob(std::function<void()> job);
    void waitforthread();
    void ForceCancelThreadWait();

    size_t threadswaiting();
    size_t threadsinuse();
    size_t numberofthreads();

  private:
    void threadpooltask(size_t thread_index);

    std::vector<cjthread> threads;
    std::vector<bool> threadInUse;
    std::queue<std::function<void()>> queuedJobs;
    std::recursive_mutex qrmutex;
    std::mutex jobmutex;
    std::mutex checkmutex;
    std::condition_variable cvJobAvailable;
    std::condition_variable cvCheckForFreeThread;
    std::string name = "tp";
    size_t nThreads = 0;
    bool running = true;
    bool forceCancelWait = false;
};

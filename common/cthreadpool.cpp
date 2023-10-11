#include "cthreadpool.h"
#include <chrono>
#include <spdlog/spdlog.h>

namespace {
  const constexpr size_t wait_time_ms = 100;
  const constexpr char * default_thread_name = "tp";
}

cthreadpool::cthreadpool(size_t n_threads)
  : cthreadpool(n_threads, std::string(default_thread_name))
{
}

cthreadpool::cthreadpool(const size_t && n_threads, const std::string && t_pool_name)
  : cthreadpool(n_threads, t_pool_name)
{
}

cthreadpool::cthreadpool(size_t n_threads, const std::string & t_pool_name)
{
  nThreads = n_threads;
  name = t_pool_name;
  threads.reserve(n_threads);
  for (size_t i=0; i<n_threads; i++)
  {
    threadInUse.emplace_back(false);
    const std::string thread_name = (t_pool_name + "_" + std::to_string(i));
    threads.emplace_back(thread_name, &cthreadpool::threadpooltask, this, i);
  }
}

cthreadpool::~cthreadpool()
{
  running = false;
}

void cthreadpool::addjob(std::function<void()> job)
{
  std::lock_guard<std::recursive_mutex> lock(qrmutex);
  queuedJobs.push(job);
  cvJobAvailable.notify_all();
}

void cthreadpool::waitforthread()
{
  std::unique_lock<decltype(checkmutex)> lock_check (checkmutex);
  while (!forceCancelWait && cvCheckForFreeThread.wait_for(lock_check , std::chrono::milliseconds(100), [this]() -> bool {
    return (!queuedJobs.empty());
  }));

  forceCancelWait = false;
}

void cthreadpool::ForceCancelThreadWait()
{
  forceCancelWait = true;
}

size_t cthreadpool::threadswaiting()
{
  size_t n_waiting = 0;
  for (const auto & t : threadInUse)
  {
    if (!t)
    {
      n_waiting++;
    }
  }

  return n_waiting;
}

size_t cthreadpool::threadsinuse()
{
  size_t n_used = 0;
  for (const auto & t : threadInUse)
  {
    if (t)
    {
      n_used++;
    }
  }

  return n_used;
}

size_t cthreadpool::numberofthreads()
{
  return nThreads;
}

void cthreadpool::threadpooltask(size_t thread_index)
{
  std::function<void()> job;

  while(running)
  {
    {
      std::lock_guard<std::recursive_mutex> lock(qrmutex);
      if (!queuedJobs.empty())
      {
        job = queuedJobs.front();
        queuedJobs.pop();
        threadInUse[thread_index] = true;
      }
    }

    if (job)
    {
      cvCheckForFreeThread.notify_all();
      job();
      job = nullptr;
      threadInUse[thread_index] = false;
    }

    // TODO: needs to be reworked as this is slowing the processing down
    std::unique_lock<std::mutex> job_lock(jobmutex);
    cvJobAvailable.wait_for(job_lock, std::chrono::milliseconds(wait_time_ms), [this]() {return !queuedJobs.empty();});

  }
}

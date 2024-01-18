#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <thread>

#include "queue_types.h"

class ThreadPool {
 public:
  ThreadPool(TasksQueue &tasks, size_t numThreads);

  void Stop();
  void Join();

 private:
  TasksQueue &tasks_;

  std::vector<std::thread> threads_;
  std::atomic_bool need_stop_;
};

#endif  // THREAD_POOL_H
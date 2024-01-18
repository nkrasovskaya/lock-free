#include "thread_pool.h"

ThreadPool::ThreadPool(TasksQueue &tasks, size_t numThreads)
    : tasks_(tasks), need_stop_(false) {
  for (size_t i = 0; i < numThreads; ++i) {
    threads_.emplace_back([this, i] {
      while (true) {
        std::function<void()> task;
#ifdef LOCK_FREE
        while (!tasks_.TryDequeue(task)) {
          if (need_stop_) {
            return;
          }
        }
#else   // LOCK_FREE
        if (!tasks_.Dequeue(task)) {
          return;
        }
#endif  // LOCK_FREE
        task();
      }
    });
  }
}

void ThreadPool::Stop() {
  need_stop_ = true;
#ifndef LOCK_FREE
  tasks_.Stop();
#endif
}

void ThreadPool::Join() {
  for (std::thread &thread : threads_) {
    thread.join();
  }
}
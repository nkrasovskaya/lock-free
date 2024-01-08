#include "thread_pool.h"

ThreadPool::ThreadPool(size_t numThreads, size_t buff_size)
    : tasks_(buff_size), need_stop_(false) {
  for (size_t i = 0; i < numThreads; ++i) {
    threads_.emplace_back([this, i] {
      while (true) {
        std::function<void()> task;
#ifdef LOCK_FREE
        while (!tasks_.tryPop(task)) {
          if (need_stop_) {
            return;
          }
        }
#else   // LOCK_FREE
        if (!tasks_.pop(task)) {
          return;
        }
#endif  // LOCK_FREE
        task();
      }
    });
  }
}

void ThreadPool::stop() {
  need_stop_ = true;
#ifndef LOCK_FREE
  tasks_.stop();
#endif
}

void ThreadPool::join() {
  for (std::thread &thread : threads_) {
    thread.join();
  }
}
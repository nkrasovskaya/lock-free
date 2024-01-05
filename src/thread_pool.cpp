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
#else // LOCK_FREE
        {
          std::unique_lock<std::mutex> lock(mutex_);
          condition_.wait(lock, [this] {
            return std::forward<bool>(need_stop_) || !tasks_.empty();
          });
          if (need_stop_ && tasks_.empty()) {
            return;
          }
          tasks_.pop(task);
        }
#endif // LOCK_FREE
        task();
      }
    });
  }
}

void ThreadPool::stop() {
  if (!need_stop_) {
    need_stop_ = true;
#ifndef LOCK_FREE
    condition_.notify_all();
#endif // LOCK_FREE
  }
}

ThreadPool::~ThreadPool() {
  if (!need_stop_) {
    stop();
  }

  for (std::thread &thread : threads_) {
    thread.join();
  }
}
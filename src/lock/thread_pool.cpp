#include "lock/thread_pool.h"

namespace locks {

ThreadPool::ThreadPool(size_t numThreads) : need_stop_(false) {
  for (size_t i = 0; i < numThreads; ++i) {
    threads_.emplace_back([this, i] {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(mutex_);
          condition_.wait(lock, [this] {
            return std::forward<bool>(need_stop_) || !tasks_.empty();
          });
          if (need_stop_ && tasks_.empty()) {
            return;
          }
          task = std::move(tasks_.front());
          tasks_.pop();
        }
        task();
      }
    });
  }
}

void ThreadPool::stop() {
  if (!need_stop_) {
    need_stop_ = true;
    condition_.notify_all();
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

}  // namespace locks
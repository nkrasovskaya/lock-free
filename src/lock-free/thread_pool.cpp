#include "lock-free/thread_pool.h"

namespace lock_free {

ThreadPool::ThreadPool(size_t numThreads) : need_stop_(false) {
  for (size_t i = 0; i < numThreads; ++i) {
    threads_.emplace_back([this, i] {
      while (true) {
        std::function<void()> task;
        {
          if (need_stop_ /* && tasks.empty()*/) {
            return;
          }
          tasks_.pop(task);
        }
        task();
      }
    });
  }
}

void ThreadPool::stop() {
  if (!need_stop_) {
    need_stop_ = true;
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

}  // namespace lock_free
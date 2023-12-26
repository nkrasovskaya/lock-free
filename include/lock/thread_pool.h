#ifndef LOCK_THREAD_POOL_H
#define LOCK_THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>

#include "ring_buffer.h"

namespace locks {

class ThreadPool {
 public:
  ThreadPool(size_t numThreads);

  template <class F, class... Args>
  void addTask(F &&f, Args &&...args) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      tasks_.push([f = std::forward<F>(f),
                   args = std::make_tuple(std::forward<Args>(args)...)] {
        std::apply(f, args);
      });
    }
    condition_.notify_one();
  }

  void stop();

  ~ThreadPool();

 private:
  std::vector<std::thread> threads_;
  locks::RingBuffer<std::function<void()>, 128> tasks_;
  std::mutex mutex_;
  std::condition_variable condition_;
  std::atomic_bool need_stop_;
};

}  // namespace locks

#endif  // LOCK_THREAD_POOL_H
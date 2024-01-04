#ifndef LOCK_FREE_THREAD_POOL_H
#define LOCK_FREE_THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>

#include "lock-free/ring_buffer.h"

namespace lock_free {

class ThreadPool {
 public:
  ThreadPool(size_t numThreads, size_t buff_size);

  template <class F, class... Args>
  void addTask(F &&f, Args &&...args) {
    {
      tasks_.push([f = std::forward<F>(f),
                   args = std::make_tuple(std::forward<Args>(args)...)] {
        std::apply(f, args);
      });
    }
  }

  void stop();

  ~ThreadPool();

 private:
  std::vector<std::thread> threads_;
  RingBuffer<std::function<void()>> tasks_;

  std::atomic_bool need_stop_;
};

}  // namespace lock_free

#endif  // LOCK_FREE_THREAD_POOL_H
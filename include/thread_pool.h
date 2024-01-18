#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>

#ifdef LOCK_FREE
#include "lock-free/ring_buffer.h"
#else  // LOCK_FREE
#include "lock/ring_buffer.h"
#endif  // LOCK_FREE

class ThreadPool {
 public:
  ThreadPool(size_t numThreads, size_t buff_size);

  template <class F, class... Args>
  void AddTask(F &&f, Args &&...args) {
    {
      tasks_.Enqueue([f = std::forward<F>(f),
                   args = std::make_tuple(std::forward<Args>(args)...)] {
        std::apply(f, args);
      });
    }
  }

  void Stop();
  void Join();

 private:
#ifdef LOCK_FREE
  lock_free::RingBuffer<std::function<void()>> tasks_;
#else   // LOCK_FREE
  locks::RingBufferThreadSafe<std::function<void()>> tasks_;  
#endif  // LOCK_FREE

  std::vector<std::thread> threads_;
  std::atomic_bool need_stop_;
};

#endif  // THREAD_POOL_H
#ifndef LOCK_FREE_TASK_GENERATOR_H
#define LOCK_FREE_TASK_GENERATOR_H

#include <atomic>
#include <thread>

namespace lock_free {

class ThreadPool;
class Logger;

class TaskGenerator {
 public:
  TaskGenerator(ThreadPool &threadPool, Logger &logger);

  TaskGenerator(const TaskGenerator &) = delete;

  ~TaskGenerator();

  void stop() { need_stop_ = true; }

 private:
  ThreadPool &thread_pool_;
  lock_free::Logger &logger_;

  std::atomic_int task_counter_;

  std::atomic_bool need_stop_;
  std::thread thread;

  void printCounter();
};

}  // namespace lock_free

#endif  // LOCK_FREE_TASK_GENERATOR_H
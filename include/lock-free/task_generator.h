#ifndef LOCK_FREE_TASK_GENERATOR_H
#define LOCK_FREE_TASK_GENERATOR_H

#include <atomic>

namespace lock_free {

class ThreadPool;
class Logger;

class TaskGenerator {
 public:
  TaskGenerator(ThreadPool &threadPool, Logger &logger);

  TaskGenerator(const TaskGenerator &) = delete;

  void run();

  void stop() { need_stop_ = true; }

 private:
  ThreadPool &thread_pool_;
  lock_free::Logger &logger_;
  std::atomic_int task_counter_;
  std::atomic_bool need_stop_;

  void printCounter();
};

}  // namespace lock_free

#endif  // LOCK_FREE_TASK_GENERATOR_H
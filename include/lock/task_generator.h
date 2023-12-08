#ifndef LOCK_TASK_GENERATOR_H
#define LOCK_TASK_GENERATOR_H

#include <atomic>

namespace locks {

class ThreadPool;

class TaskGenerator {
 public:
  TaskGenerator(ThreadPool &threadPool);

  TaskGenerator(const TaskGenerator &) = delete;

  void run();

  void stop() { need_stop_ = true; }

 private:
  ThreadPool &thread_pool_;
  std::atomic_int task_counter_;
  std::atomic_bool need_stop_;

  void printCounter();
};

}  // namespace locks

#endif  // LOCK_TASK_GENERATOR_H
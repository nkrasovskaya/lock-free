#ifndef TASK_GENERATOR_H
#define TASK_GENERATOR_H

#include <atomic>
#include <thread>

class ThreadPool;
class Logger;

class TaskGenerator {
 public:
  TaskGenerator(ThreadPool &threadPool, Logger &logger, size_t tasks_num);

  TaskGenerator(const TaskGenerator &) = delete;

  ~TaskGenerator();

  void stop() { need_stop_ = true; }

 private:
  ThreadPool &thread_pool_;
  Logger &logger_;

  std::atomic_int task_counter_;

  std::atomic_bool need_stop_;
  std::thread thread;

  void printCounter();
};

#endif  // TASK_GENERATOR_H
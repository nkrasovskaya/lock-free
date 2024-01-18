#ifndef TASK_GENERATOR_H
#define TASK_GENERATOR_H

#include <atomic>
#include <thread>
#include <vector>

class ThreadPool;
class Logger;

class TaskGenerator {
 public:
  TaskGenerator(size_t numThreads, ThreadPool &threadPool, Logger &logger,
                size_t max_tasks_num, std::atomic_int &task_counter);

  TaskGenerator(const TaskGenerator &) = delete;

  void Stop();
  void Join();

 private:
  ThreadPool &thread_pool_;
  Logger &logger_;

  std::atomic_size_t gen_tasks_;
  std::atomic_int &task_counter_;
  size_t max_tasks_num_;

  std::vector<std::thread> threads_;
  std::atomic_bool need_stop_;
};

#endif  // TASK_GENERATOR_H
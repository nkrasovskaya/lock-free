#ifndef TASK_GENERATOR_H
#define TASK_GENERATOR_H

#include <atomic>
#include <thread>
#include <vector>

#include "queue_types.h"

class Logger;

class TaskGenerator {
 public:
  TaskGenerator(size_t numThreads, TasksQueue &tasks, Logger &logger,
                size_t max_tasks_num, std::atomic_int &task_counter);

  TaskGenerator(const TaskGenerator &) = delete;

  template <class F, class... Args>
  void AddTask(F &&f, Args &&...args) {
    tasks_.Enqueue([f = std::forward<F>(f),
                    args = std::make_tuple(std::forward<Args>(args)...)] {
      std::apply(f, args);
    });
  }

  void Stop();
  void Join();

 private:
  TasksQueue &tasks_;
  Logger &logger_;

  std::atomic_size_t gen_tasks_;
  std::atomic_int &task_counter_;
  size_t max_tasks_num_;

  std::vector<std::thread> threads_;
  std::atomic_bool need_stop_;
};

#endif  // TASK_GENERATOR_H
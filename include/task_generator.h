#ifndef TASK_GENERATOR_H
#define TASK_GENERATOR_H

#include <atomic>
#include <thread>

#include "runnable.h"

class ThreadPool;
class Logger;

class TaskGenerator : public Runnable {
 public:
  TaskGenerator(ThreadPool &threadPool, Logger &logger, size_t max_tasks_num,
                std::atomic_int &task_counter)
      : Runnable(),
        thread_pool_(threadPool),
        logger_(logger),
        max_tasks_num_(max_tasks_num),
        task_counter_(task_counter) {}

  TaskGenerator(const TaskGenerator &) = delete;

 private:
  ThreadPool &thread_pool_;
  Logger &logger_;

  size_t max_tasks_num_;
  std::atomic_int &task_counter_;

  void run() override;
};

#endif  // TASK_GENERATOR_H
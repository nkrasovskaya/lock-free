#include "task_generator.h"

#include <cmath>
#include <iostream>

#include "logger.h"
#include "thread_pool.h"

namespace {
double integrate(double a, double b) {
  const int min = 100000;
  const int max = 1000000;
  const int N = min + (rand() % static_cast<int>(max - min + 1));

  double h = (b - a) / N;
  double sum = (sin(a) + sin(b)) / 2.0;

  for (int i = 1; i < N; ++i) {
    double x = a + i * h;
    sum += sin(x);
  }

  return sum * h;
}
}  // namespace

TaskGenerator::TaskGenerator(ThreadPool &threadPool, Logger &logger,
                             size_t tasks_num)
    : thread_pool_(threadPool),
      logger_(logger),
      task_counter_(0),
      need_stop_(false) {
  thread = std::move(std::thread([this, tasks_num] {
    for (size_t i = 0;
         i < (tasks_num == 0 ? std::numeric_limits<size_t>::max() : tasks_num);
         ++i) {
      double a = static_cast<double>(rand()) / RAND_MAX;
      double b = static_cast<double>(rand()) / RAND_MAX;
      thread_pool_.addTask([this, a, b] {
        ++task_counter_;
        auto ts = std::chrono::high_resolution_clock::now();
        double result = integrate(a, b);
        auto te = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> ms_double = te - ts;

        std::unique_ptr<LogMessage> log_message(new LogMessage);
        log_message->set_time();
        log_message->fname = __FILE__;
        log_message->line_num = __LINE__;
        log_message->smsg << "a: " << a << ", b: " << b
                          << ", result: " << result
                          << ", execution time: " << ms_double;

        logger_.addMessage(std::move(log_message));
      });

      if (need_stop_) {
        thread_pool_.stop();
        logger_.stop();
        break;
      }
    }
  }));
}

TaskGenerator::~TaskGenerator() {
  thread.join();
  printCounter();
}

void TaskGenerator::printCounter() {
  std::cout << "Tasks number: " << task_counter_ << std::endl;
}
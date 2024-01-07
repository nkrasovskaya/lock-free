#include "task_generator.h"

#include <cmath>

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
                             size_t tasks_num, std::atomic_int &task_counter)
    : thread_pool_(threadPool),
      logger_(logger),
      task_counter_(task_counter),
      need_stop_(false) {
  thread = std::move(std::thread([this, tasks_num] {
    for (size_t i = 0;
         i < (tasks_num == 0 ? std::numeric_limits<size_t>::max() : tasks_num);
         ++i) {
      double a = static_cast<double>(rand()) / RAND_MAX;
      double b = static_cast<double>(rand()) / RAND_MAX;
      thread_pool_.addTask([this, a, b, i] {
        ++task_counter_;

        auto ts = std::chrono::high_resolution_clock::now();
        double result = integrate(a, b);
        auto te = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> ms_double = te - ts;

        std::unique_ptr<LogMessage> log_message(new LogMessage);
        log_message->set_time();
        log_message->fname = __FILE__;
        log_message->line_num = __LINE__;
        log_message->smsg << "a: " << a << ", b: " << b << ", num: " << i
                          << ", result: " << result
                          << ", execution time: " << ms_double;

        logger_.addMessage(std::move(log_message));
      });

      if (need_stop_) {
        break;
      }
    }
  }));
}

TaskGenerator::~TaskGenerator() {
  thread.join();
  thread_pool_.stop();
  logger_.stop();
}
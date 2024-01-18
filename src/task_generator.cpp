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

TaskGenerator::TaskGenerator(size_t numThreads, TasksQueue &tasks,
                             Logger &logger, size_t max_tasks_num,
                             std::atomic_int &task_counter)
    : tasks_(tasks),
      logger_(logger),
      gen_tasks_(0),
      task_counter_(task_counter),
      max_tasks_num_(max_tasks_num == 0 ? std::numeric_limits<size_t>::max()
                                        : max_tasks_num) {
  for (size_t i = 0; i < numThreads; ++i) {
    threads_.emplace_back([this] {
      while (gen_tasks_ < max_tasks_num_) {
        size_t tnum = gen_tasks_++;
        auto ts = std::chrono::high_resolution_clock::now();
        const int sleep_time = rand() % 8;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));

        double a = static_cast<double>(rand()) / RAND_MAX;
        double b = static_cast<double>(rand()) / RAND_MAX;
        AddTask([this, a, b, tnum] {
          ++task_counter_;

          auto ts = std::chrono::high_resolution_clock::now();
          const int sleep_time = rand() % 100;
          double result = integrate(a, b);
          auto te = std::chrono::high_resolution_clock::now();

          std::chrono::duration<double, std::milli> ms_double = te - ts;

          std::unique_ptr<LogMessage> log_message(new LogMessage);
          log_message->set_time();
          log_message->fname = __FILE__;
          log_message->line_num = __LINE__;
          log_message->smsg << "a: " << a << ", b: " << b << ", num: " << tnum
                            << ", result: " << result
                            << ", execution time: " << ms_double;

          logger_.AddMessage(std::move(log_message));
        });

        // if (IsNeedStop()) {
        //   break;
        // }
      }
    });
  }
}

void TaskGenerator::Stop() { need_stop_ = true; }

void TaskGenerator::Join() {
  for (std::thread &thread : threads_) {
    thread.join();
  }
}
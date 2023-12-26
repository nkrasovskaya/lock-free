#include "lock/task_generator.h"

#include <cmath>
#include <iostream>

#include "lock/thread_pool.h"

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

namespace locks {

TaskGenerator::TaskGenerator(ThreadPool &threadPool)
    : thread_pool_(threadPool), task_counter_(0), need_stop_(false) {}

void TaskGenerator::run() {
  while (true) {
    double a = static_cast<double>(rand()) / RAND_MAX;
    double b = static_cast<double>(rand()) / RAND_MAX;
    thread_pool_.addTask([this, a, b] {
      double result = integrate(a, b);
      std::cout << "result: " << result << std::endl;
      ++task_counter_;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (need_stop_) {
      thread_pool_.stop();
      break;
    }
  }

  printCounter();
}

void TaskGenerator::printCounter() {
  std::cout << "Tasks number: " << task_counter_ << std::endl;
}

}  // namespace locks
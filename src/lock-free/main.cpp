#include <iostream>

#include "lock-free/logger.h"
#include "lock-free/task_generator.h"
#include "lock-free/thread_pool.h"

template <class T>
concept HasStopMethod = requires(T a) { a.stop(); };

template <HasStopMethod T>
class StopOnPressEnter final {
 public:
  StopOnPressEnter(T &runner) {
    t = std::move(std::thread([&runner] {
      while (true) {
        if (std::cin.get() == '\n') {
          runner.stop();
          break;
        }
      }
    }));
  }

  ~StopOnPressEnter() { t.join(); }

 private:
  std::thread t;
};

int main() {
  const size_t numThreads = 10;
  lock_free::ThreadPool threadPool(numThreads);
  lock_free::Logger logger(new lock_free::FileLogAppender("./test.log"));
  lock_free::TaskGenerator taskGenerator(threadPool, logger);
  StopOnPressEnter<lock_free::TaskGenerator> se(
      std::forward<lock_free::TaskGenerator &>(taskGenerator));
  logger.run();
  taskGenerator.run();
  return 0;
}
#include <iostream>

#include "lock/task_generator.h"
#include "lock/thread_pool.h"
#include "lock/logger.h"

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
  locks::ThreadPool threadPool(numThreads);
  locks::Logger logger(new locks::FileLogAppender("./test.log"));
  locks::TaskGenerator taskGenerator(threadPool, logger);
  StopOnPressEnter<locks::TaskGenerator> se(
      std::forward<locks::TaskGenerator &>(taskGenerator));
  logger.run();
  taskGenerator.run();
  return 0;
}
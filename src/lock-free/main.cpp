#include <iostream>

#include "config.h"
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

int main(int argc, char *argv[]) {
  Config config;
  if (argc <= 1) {
    std::cout << "No config file. Use default variables" << std::endl;
  } else {
    std::ifstream is(argv[1]);
    if (!is.good()) {
      std::cerr << "Can't open configuration file" << std::endl;
      return 1;
    }
    try {
      config.Parse(&is);
    } catch (std::exception &e) {
      std::cerr << "Can't load configuration file: " << e.what() << std::endl;
      return 1;
    } catch (...) {
      std::cerr << "Can't load configuration file: unknown error" << std::endl;
      return 1;
    }
  }

  lock_free::ThreadPool threadPool(config.GetThreadsNumber());
  lock_free::Logger logger(new lock_free::FileLogAppender(config.GetLogFilePath()));
  lock_free::TaskGenerator taskGenerator(threadPool, logger);
  StopOnPressEnter<lock_free::TaskGenerator> se(
      std::forward<lock_free::TaskGenerator &>(taskGenerator));
  logger.run();
  taskGenerator.run();
  return 0;
}
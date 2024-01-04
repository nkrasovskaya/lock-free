#include <iostream>

#include "config.h"
#include "lock/logger.h"
#include "lock/task_generator.h"
#include "lock/thread_pool.h"

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

  locks::ThreadPool threadPool(config.GetThreadsNumber(),
                               config.GetTasksBufferSize());
  locks::Logger logger(new locks::FileLogAppender(config.GetLogFilePath()),
                       config.GetLogBufferSize());
  locks::TaskGenerator taskGenerator(threadPool, logger);

  StopOnPressEnter<locks::TaskGenerator> se(
      std::forward<locks::TaskGenerator &>(taskGenerator));

  taskGenerator.run();
  return 0;
}
#include <iostream>

#include "config.h"
#include "logger.h"
#include "task_generator.h"
#include "thread_pool.h"

int main(int argc, char *argv[]) {
#ifdef LOCK_FREE
  std::cout << "lock free" << std::endl;
#else  // LOCK_FREE
  std::cout << "lock" << std::endl;
#endif

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

  std::cout << "Threads number: " << config.GetThreadsNumber() << std::endl;
  std::cout << "Tasks buffer size: " << config.GetTasksBufferSize()
            << std::endl;
  std::cout << "Log buffer size: " << config.GetLogBufferSize() << std::endl;
  std::cout << "Tasks number: " << config.GetTasksNumber() << std::endl;
  std::cout << "Log file path: " << config.GetLogFilePath() << std::endl;

  std::atomic_int task_counter;

  auto ts = std::chrono::high_resolution_clock::now();
  Logger logger(new FileLogAppender(config.GetLogFilePath()),
                config.GetLogBufferSize());
  logger.start();

  ThreadPool thread_pool(config.GetThreadsNumber(),
                         config.GetTasksBufferSize());

  TaskGenerator task_generator(thread_pool, logger, config.GetTasksNumber(),
                               task_counter);
  task_generator.start();

  if (config.GetTasksNumber() == 0) {
    while (true) {
      if (std::cin.get() == '\n') {
        task_generator.stop();
        break;
      }
    }
  }

  task_generator.join();
  thread_pool.stop();
  thread_pool.join();
  logger.stop();
  logger.join();

  auto te = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> ms_double = te - ts;
  std::cout << "Execution time: " << ms_double << std::endl;

  std::cout << "Tasks number: " << task_counter << std::endl;

  return 0;
}
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

  std::cout << "Thread pool threads number: " << config.GetThreadsNumber()
            << std::endl;
  std::cout << "Task generator threads number: "
            << config.GetTaskGeneratorThreadNumber() << std::endl;
  std::cout << "Tasks buffer size: " << config.GetTasksBufferSize()
            << std::endl;
  std::cout << "Log buffer size: " << config.GetLogBufferSize() << std::endl;
  std::cout << "Tasks number: " << config.GetTasksNumber() << std::endl;
  std::cout << "Log file path: " << config.GetLogFilePath() << std::endl;

  std::atomic_int task_counter;

  auto ts = std::chrono::high_resolution_clock::now();
  LoggerQueue logger_queue(config.GetLogBufferSize());
  Logger logger(logger_queue, new FileLogAppender(config.GetLogFilePath()));
  logger.Start();

#ifdef LOCK_FREE
  TasksQueue tasks_queue(config.GetThreadsNumber() +
                         config.GetTaskGeneratorThreadNumber());
#else
  TasksQueue tasks_queue;
#endif

  ThreadPool thread_pool(tasks_queue, config.GetThreadsNumber());

  TaskGenerator task_generator(config.GetTaskGeneratorThreadNumber(),
                               tasks_queue, logger, config.GetTasksNumber(),
                               task_counter);

  if (config.GetTasksNumber() == 0) {
    while (true) {
      if (std::cin.get() == '\n') {
        task_generator.Stop();
        break;
      }
    }
  }

  task_generator.Join();
  thread_pool.Stop();
  thread_pool.Join();
  logger.Stop();
  logger.Join();

  auto te = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> ms_double = te - ts;
  std::cout << "Execution time: " << ms_double << std::endl;

  std::cout << "Tasks number: " << task_counter << std::endl;

  return 0;
}
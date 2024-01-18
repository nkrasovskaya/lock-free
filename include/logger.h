#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <fstream>
#include <memory>
#include <sstream>

#include "queue_types.h"
#include "runnable.h"

struct LogMessage {
  time_t time;
  std::string fname;
  int line_num;
  std::stringstream smsg;

  void set_time() {
    auto now = std::chrono::system_clock::now();
    time = std::chrono::system_clock::to_time_t(now);
  }
};

template <typename T>
LogMessage& operator<<(LogMessage& log_msg, const T& s) {
  log_msg.smsg << s;
  return log_msg;
}

class LogAppender {
 public:
  LogAppender() = default;
  LogAppender(const LogAppender&) = delete;
  virtual ~LogAppender() {}

  virtual bool Write(std::string msg) = 0;
};

class FileLogAppender final : public LogAppender {
 public:
  FileLogAppender(std::string file_path);
  ~FileLogAppender();

  bool Write(std::string msg) override;

 private:
  std::ofstream log_file_;
};

class Logger : public Runnable {
 public:
  Logger(LoggerQueue &logger_queue, LogAppender* helper);

  bool AddMessage(std::unique_ptr<LogMessage>&& msg);

  void Stop() override;

 private:
  std::unique_ptr<LogAppender> appender_;

  LoggerQueue &logger_queue_;

  void Run() override;
};

#endif  // LOGGER_H
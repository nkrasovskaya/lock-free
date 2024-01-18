#ifndef LOGGER_H
#define LOGGER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <sstream>
#include <thread>

#ifdef LOCK_FREE
#include "lock-free/ring_buffer.h"
#else  // LOCK_FREE
#include "lock/ring_buffer.h"
#endif  // LOCK_FREE

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
  Logger(LogAppender* helper, size_t buff_size);

  bool AddMessage(std::unique_ptr<LogMessage>&& msg);

  void Stop() override;

 private:
  std::unique_ptr<LogAppender> appender_;
#ifdef LOCK_FREE
  lock_free::RingBuffer<std::unique_ptr<LogMessage>> buffer_;
#else   // LOCK_FREE
  locks::RingBufferThreadSafe<std::unique_ptr<LogMessage>> buffer_;
#endif  // LOCK_FREE

  void Run() override;
};

#endif  // LOGGER_H
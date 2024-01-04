#ifndef LOCK_LOGGER_H
#define LOCK_LOGGER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <sstream>
#include <thread>

#include "lock/ring_buffer.h"

namespace locks {

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
  virtual bool write(std::string msg) = 0;
};

class FileLogAppender final : public LogAppender {
 public:
  FileLogAppender(std::string file_path);
  ~FileLogAppender();

  bool write(std::string msg) override;

 private:
  std::ofstream log_file_;
};

class Logger {
 public:
  Logger(LogAppender* helper, size_t buff_size);
  ~Logger();

  bool addMessage(std::unique_ptr<LogMessage>&& msg);
  void run();

  void stop();

 private:
  std::unique_ptr<LogAppender> helper_;
  RingBuffer<std::unique_ptr<LogMessage>> buffer_;

  std::atomic_bool need_stop_;
  std::mutex buff_lock_;
  std::condition_variable buff_is_not_full_condition_;
  std::condition_variable buff_is_not_empty_condition_;
  std::thread thread;
};
}  // namespace locks

#endif  // LOCK_LOGGER_H
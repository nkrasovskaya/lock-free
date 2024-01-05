#include "logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace {
std::string serializeLogMeassage(const LogMessage &msg) {
  std::ostringstream record_stream;
  record_stream << std::put_time(std::localtime(&msg.time), "%Y-%m-%d %H:%M:%S")
                << "  " << msg.fname << ":" << msg.line_num << " "
                << msg.smsg.str() << std::endl;

  return record_stream.str();
}
}  // namespace

FileLogAppender::FileLogAppender(std::string file_path)
    : log_file_(file_path) {}
FileLogAppender::~FileLogAppender() {
  log_file_.flush();
  log_file_.close();
}

bool FileLogAppender::write(std::string msg) {
  log_file_.write(msg.data(), msg.size());
  return true;
}

Logger::Logger(LogAppender *helper, size_t buff_size)
    : appender_(helper), buffer_(buff_size), need_stop_(false) {
  thread = std::move(std::thread([this] {
    std::unique_ptr<LogMessage> msg;
    while (true) {
#ifdef LOCK_FREE
      if (need_stop_) {
        return;
      }
      buffer_.pop(msg);
      appender_->write(serializeLogMeassage(*msg));
#else // LOCK_FREE
      {
        std::unique_lock<std::mutex> lock(buff_lock_);
        buff_is_not_empty_condition_.wait(lock, [this] {
          return std::forward<bool>(need_stop_) || !buffer_.empty();
        });
        if (need_stop_ && buffer_.empty()) {
          return;
        }
        buffer_.pop(msg);
      }
      buff_is_not_full_condition_.notify_one();
      appender_->write(serializeLogMeassage(*msg));
#endif // LOCK_FREE
    }
  }));
}

void Logger::stop() {
  if (!need_stop_) {
    need_stop_ = true;

#ifdef LOCK_FREE
    // Add fake message to exit from read loop
    std::unique_ptr<LogMessage> log_message(new LogMessage);
    log_message->set_time();
    log_message->fname = __FILE__;
    log_message->line_num = __LINE__;
    log_message->smsg << "Stopping logger...";
    addMessage(std::move(log_message));
#else // LOCK_FREE
    buff_is_not_empty_condition_.notify_all();
    buff_is_not_full_condition_.notify_all();
#endif // LOCK_FREE
  }
}

Logger::~Logger() {
  if (!need_stop_) {
    stop();
  }

  thread.join();
}

bool Logger::addMessage(std::unique_ptr<LogMessage> &&msg) {
#ifdef LOCK_FREE
  buffer_.push(std::move(msg));
#else // LOCK_FREE
  {
    std::unique_lock<std::mutex> lock(buff_lock_);
    buff_is_not_full_condition_.wait(lock, [this] {
      return std::forward<bool>(need_stop_) || !buffer_.full();
    });
    if (need_stop_) {
      return true;
    }
    buffer_.push(std::move(msg));
  }

  buff_is_not_empty_condition_.notify_one();
#endif // LOCK_FREE

  return true;
}
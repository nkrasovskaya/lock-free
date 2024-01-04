#include "lock/logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace {
std::string serializeLogMeassage(const locks::LogMessage &msg) {
  std::ostringstream record_stream;
  record_stream << std::put_time(std::localtime(&msg.time), "%Y-%m-%d %H:%M:%S")
                << "  " << msg.fname << ":" << msg.line_num << " " << msg.smsg.str()
                << std::endl;

  return record_stream.str();
}
}  // namespace

namespace locks {

FileLogAppender::FileLogAppender(std::string file_path) : log_file_(file_path) {}
FileLogAppender::~FileLogAppender() {
  log_file_.flush();
  log_file_.close();
}

bool FileLogAppender::write(std::string msg) {
  log_file_.write(msg.data(), msg.size());
  return true;
}

Logger::Logger(LogAppender *helper) : helper_(helper), need_stop_(false) {}

void Logger::stop() {
  if (!need_stop_) {
    need_stop_ = true;
    buff_is_not_empty_condition_.notify_all();
    buff_is_not_full_condition_.notify_all();
  }
}

Logger::~Logger() {
  if (!need_stop_) {
    stop();
  }

  thread.join();
}

bool Logger::addMessage(std::unique_ptr<LogMessage> &&msg) {
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
  return true;
}

// move to constructor
void Logger::run() {
  thread = std::move(std::thread([this] {
    std::unique_ptr<LogMessage> msg;
    while (true) {
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
      helper_->write(serializeLogMeassage(*msg));
    }
  }));
}
}  // namespace locks
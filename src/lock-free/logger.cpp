#include "lock-free/logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace {
std::string serializeLogMeassage(const lock_free::LogMessage &msg) {
  std::ostringstream record_stream;
  record_stream << std::put_time(std::localtime(&msg.time), "%Y-%m-%d %H:%M:%S")
                << "  " << msg.fname << ":" << msg.line_num << " "
                << msg.smsg.str() << std::endl;

  return record_stream.str();
}
}  // namespace

namespace lock_free {

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
  }
}

Logger::~Logger() {
  if (!need_stop_) {
    stop();
  }

  t.join();
}

bool Logger::addMessage(std::unique_ptr<LogMessage> &&msg) {
  {
    if (need_stop_) {
      return true;
    }
    buffer_.push(std::move(msg));
  }
  return true;
}

// move to constructor
void Logger::run() {
  t = std::move(std::thread([this] {
    std::unique_ptr<LogMessage> msg;
    while (true) {
      if (need_stop_ /* && tasks.empty()*/) {
        return;
      }
      buffer_.pop(msg);
      helper_->write(serializeLogMeassage(*msg));
    }
  }));
}
}  // namespace lock_free
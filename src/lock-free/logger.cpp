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
    : helper_(helper), buffer_(buff_size), need_stop_(false) {}

void Logger::stop() {
  if (!need_stop_) {
    need_stop_ = true;

    // Add fake message to exit from read loop
    std::unique_ptr<LogMessage> log_message(new LogMessage);
    log_message->set_time();
    log_message->fname = __FILE__;
    log_message->line_num = __LINE__;
    log_message->smsg << "Stopping logger...";
    addMessage(std::move(log_message));
  }
}

Logger::~Logger() {
  if (!need_stop_) {
    stop();
  }

  thread.join();
}

bool Logger::addMessage(std::unique_ptr<LogMessage> &&msg) {
  buffer_.push(std::move(msg));
  return true;
}

// move to constructor
void Logger::run() {
  thread = std::move(std::thread([this] {
    std::unique_ptr<LogMessage> msg;
    while (true) {
      if (need_stop_) {
        return;
      }
      buffer_.pop(msg);
      helper_->write(serializeLogMeassage(*msg));
    }
  }));
}
}  // namespace lock_free
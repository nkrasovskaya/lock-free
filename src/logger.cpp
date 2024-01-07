#include "logger.h"

#include <chrono>
#include <iomanip>
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
#else  // LOCK_FREE
      if (!buffer_.pop(msg)) {
        return;
      }

#endif  // LOCK_FREE
      appender_->write(serializeLogMeassage(*msg));
    }
  }));
}

void Logger::stop() {
  need_stop_ = true;

#ifdef LOCK_FREE
  // Add fake message to exit from read loop
  std::unique_ptr<LogMessage> log_message(new LogMessage);
  log_message->set_time();
  log_message->fname = __FILE__;
  log_message->line_num = __LINE__;
  log_message->smsg << "Stopping logger...";
  addMessage(std::move(log_message));
#else   // LOCK_FREE
  buffer_.stop();
#endif  // LOCK_FREE
  thread.join();
}

bool Logger::addMessage(std::unique_ptr<LogMessage> &&msg) {
  buffer_.push(std::move(msg));

  return true;
}
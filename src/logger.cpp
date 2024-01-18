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

bool FileLogAppender::Write(std::string msg) {
  log_file_.write(msg.data(), msg.size());
  return true;
}

Logger::Logger(LogAppender *helper, size_t buff_size)
    : Runnable(), appender_(helper), buffer_(buff_size) {}

bool Logger::AddMessage(std::unique_ptr<LogMessage> &&msg) {
  buffer_.Enqueue(std::move(msg));

  return true;
}

void Logger::Stop() {
  Runnable::Stop();

#ifndef LOCK_FREE
  buffer_.Stop();
#endif  // LOCK_FREE
}

void Logger::Run() {
  std::unique_ptr<LogMessage> msg;
  while (true) {
#ifdef LOCK_FREE
    while (!buffer_.TryDequeue(msg)) {
      if (IsNeedStop()) {
        return;
      }
    }
#else  // LOCK_FREE
    if (!buffer_.Dequeue(msg)) {
      return;
    }

#endif  // LOCK_FREE
    appender_->Write(serializeLogMeassage(*msg));
  }
}
#ifndef CONFIG_H
#define CONFIG_H

#include <memory>
#include <string>

class Config {
 public:
  Config();
  ~Config();

  void Parse(std::istream *is); // throw: std::invalid_argument

  size_t GetThreadsNumber() const { return threads_number_; }
  size_t GetTasksBufferSize() const { return tasks_buffer_size_; }
  size_t GetLogBufferSize() const { return log_buffer_size_; }
  size_t GetTasksNumber() const { return tasks_number_; }
  const std::string &GetLogFilePath() const { return log_file_path_; }

 private:
  Config(const Config &) = delete;
  Config(Config &&) = delete;
  Config& operator=(const Config &) = delete;
  Config& operator=(Config &&) = delete;

  size_t threads_number_ = 16;
  size_t tasks_buffer_size_ = 128;
  size_t log_buffer_size_ = 256;
  size_t tasks_number_ = 1000;
  std::string log_file_path_;

  class ConfigImpl;
  std::unique_ptr<ConfigImpl> impl_;

  friend class ConfigImpl;
};

#endif // CONFIG_H

#ifndef CONFIG_H
#define CONFIG_H

#include <memory>
#include <string>

class Config {
 public:
  Config();
  ~Config();

  void Parse(std::istream *is); // throw: std::invalid_argument

  int GetThreadsNumber() const { return threads_number_; }
  int GetTasksBufferSize() const { return tasks_buffer_size_; }
  int GetLogBufferSize() const { return log_buffer_size_; }
  int GetTasksNumber() const { return tasks_number_; }
  const std::string &GetLogFilePath() const { return log_file_path_; }

 private:
  Config(const Config &) = delete;
  Config(Config &&) = delete;
  Config& operator=(const Config &) = delete;
  Config& operator=(Config &&) = delete;

  int threads_number_ = 16;
  int tasks_buffer_size_ = 128;
  int log_buffer_size_ = 256;
  int tasks_number_ = 1000;
  std::string log_file_path_;

  class ConfigImpl;
  std::unique_ptr<ConfigImpl> impl_;

  friend class ConfigImpl;
};

#endif // CONFIG_H

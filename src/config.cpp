#include "config.h"

#include <iostream>
#include <sstream>

#include "picojson.h"

namespace json = picojson;

class Config::ConfigImpl {
 public:
  ConfigImpl(Config *config) : config_(config) {}

  void Parse(std::istream *is);

 private:
  void ParseApp(const json::value &swos_json);

  Config *config_;
};

Config::Config() : log_file_path_("test.log"), impl_(new ConfigImpl(this)) {}

Config::~Config() = default;

void Config::Parse(std::istream *is) { impl_->Parse(is); }

// ============================================
// ********** Example of app config **********
// ============================================
//
// This config is parsed in method Config::ConfigImpl::Parse
//
//{
//  "app": {
//    "threads_number": 16,
//    "tasks_buffer_size": 128,
//    "log_buffer_size": 256,
//    "tasks_number": 1000,
//    "log_file_path": "test.log"
//  }
//}

void Config::ConfigImpl::Parse(std::istream *is) {
  std::string cfg_str((std::istreambuf_iterator<char>(*is)),
                      std::istreambuf_iterator<char>());
  if (cfg_str.empty()) {
    std::cout << "Config swos is empty" << std::endl;
    return;
  }

  std::cout << "Config:\n" << cfg_str << std::endl;

  // Parse JSON
  json::value config_json;
  std::string err = json::parse(config_json, cfg_str);
  if (!err.empty()) {
    throw std::invalid_argument(std::string("Can't parse swos config: ") + err);
  }

  if (!config_json.is<json::object>()) {
    throw std::invalid_argument("Config swos must be an object");
  }

  auto &app_json = config_json.get("app");
  if (app_json.is<json::null>()) {
    std::cout << "Config app is empty" << std::endl;
    return;
  }

  ParseApp(app_json);
}

void Config::ConfigImpl::ParseApp(const json::value &app_json) {
  if (!app_json.is<json::object>()) {
    throw std::invalid_argument("Config swos must be an object");
  }

  auto &threads_number_json = app_json.get("threads_number");
  if (!threads_number_json.is<json::null>()) {
    if (!threads_number_json.is<double>()) {
      throw std::invalid_argument("Config app threads_number must be a number");
    }

    config_->threads_number_ =
        static_cast<int>(threads_number_json.get<double>());
  }

  auto &tasks_buffer_size_json = app_json.get("tasks_buffer_size");
  if (!tasks_buffer_size_json.is<json::null>()) {
    if (!tasks_buffer_size_json.is<double>()) {
      throw std::invalid_argument("Config app tasks_buffer_size must be a number");
    }

    config_->tasks_buffer_size_ =
        static_cast<int>(tasks_buffer_size_json.get<double>());
  }

  auto &log_buffer_size_json = app_json.get("log_buffer_size");
  if (!log_buffer_size_json.is<json::null>()) {
    if (!log_buffer_size_json.is<double>()) {
      throw std::invalid_argument("Config app log_buffer_size must be a number");
    }

    config_->log_buffer_size_ =
        static_cast<int>(log_buffer_size_json.get<double>());
  }

  auto &tasks_number_json = app_json.get("tasks_number");
  if (!tasks_number_json.is<json::null>()) {
    if (!tasks_number_json.is<double>()) {
      throw std::invalid_argument("Config app tasks_number must be a number");
    }

    config_->tasks_number_ =
        static_cast<int>(tasks_number_json.get<double>());
  }

  auto &log_file_path_json = app_json.get("log_file_path");
  if (!log_file_path_json.is<json::null>()) {
    if (!log_file_path_json.is<std::string>()) {
      throw std::invalid_argument("Config app log_file_path must be a string");
    }

    config_->log_file_path_ = log_file_path_json.to_str();
  }
}

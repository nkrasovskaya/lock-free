#ifndef RUNNABLE_H
#define RUNNABLE_H

#include <atomic>
#include <thread>

class Runnable {
 public:
  Runnable() : need_stop_(false) {}
  Runnable(const Runnable &) = delete;
  virtual ~Runnable() {}

  void start() {
    thread = std::move(std::thread([this]() { run(); }));
  }

  virtual void stop() { need_stop_ = true; }

  virtual void join() {
    if (thread.joinable()) {
      thread.join();
    }
  }

 protected:
  virtual void run() = 0;
  bool isNeedStop() const { return need_stop_; }

 private:
  std::thread thread;
  std::atomic_bool need_stop_;
};

#endif  // RUNNABLE_H
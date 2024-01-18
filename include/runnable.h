#ifndef RUNNABLE_H
#define RUNNABLE_H

#include <atomic>
#include <thread>

class Runnable {
 public:
  Runnable() : need_stop_(false) {}
  Runnable(const Runnable &) = delete;
  virtual ~Runnable() {}

  void Start() {
    thread = std::move(std::thread([this]() { Run(); }));
  }

  virtual void Stop() { need_stop_ = true; }

  virtual void Join() {
    if (thread.joinable()) {
      thread.join();
    }
  }

 protected:
  virtual void Run() = 0;
  bool IsNeedStop() const { return need_stop_; }

 private:
  std::thread thread;
  std::atomic_bool need_stop_;
};

#endif  // RUNNABLE_H
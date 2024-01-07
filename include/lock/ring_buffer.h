#ifndef LOCK_RING_BUFFER_H
#define LOCK_RING_BUFFER_H

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <vector>

namespace locks {

template <typename T>
class RingBuffer {
 public:
  RingBuffer(size_t buff_size) : head_idx_(0), tail_idx_(0), is_empty_(true) {
    buffer_.resize(buff_size);
  }

  bool empty() const { return is_empty_; }

  bool full() const { return (tail_idx_ + 1) % buffer_.size() == head_idx_; }

  size_t getHeadIdx() const { return head_idx_; }
  size_t getTailIdx() const { return tail_idx_; }

  bool push(T&& data) {
    if (full()) {
      return false;
    }
    if (is_empty_) {
      head_idx_ = 0;
      tail_idx_ = 0;
      is_empty_ = false;
    } else {
      tail_idx_ = (tail_idx_ + 1) % buffer_.size();
    }
    buffer_[tail_idx_] = std::move(data);

    return true;
  }

  bool pop(T& data) {
    if (empty()) {
      return false;
    }
    data = std::move(buffer_[head_idx_]);
    if (head_idx_ == tail_idx_) {
      // last element
      is_empty_ = true;
    } else {
      head_idx_ = (head_idx_ + 1) % buffer_.size();
    }
    return true;
  }

 private:
  std::atomic_size_t head_idx_;
  std::atomic_size_t tail_idx_;
  std::atomic_bool is_empty_;

  std::vector<T> buffer_;
};

template <typename T>
class RingBufferThreadSafe {
 public:
  RingBufferThreadSafe(size_t buff_size)
      : buffer_(buff_size), need_stop_(false) {}

  ~RingBufferThreadSafe() {}

  bool push(T&& data) {
    std::unique_lock<std::mutex> lock(buff_lock_);
    buff_is_not_full_condition_.wait(lock, [this] {
      return std::forward<bool>(need_stop_) || !buffer_.full();
    });
    if (need_stop_) {
      return false;
    }

    bool res = buffer_.push(std::move(data));
    assert(res);

    buff_is_not_empty_condition_.notify_one();

    return res;
  }

  bool pop(T& data) {
    std::unique_lock<std::mutex> lock(buff_lock_);
    buff_is_not_empty_condition_.wait(lock, [this] {
      return std::forward<bool>(need_stop_) || !buffer_.empty();
    });
    if (need_stop_ && buffer_.empty()) {
      return false;
    }

    bool res = buffer_.pop(data);
    assert(res);

    buff_is_not_full_condition_.notify_one();

    return res;
  }

  void stop() {
    need_stop_ = true;
    buff_is_not_empty_condition_.notify_all();
    buff_is_not_full_condition_.notify_all();
  }

 private:
  RingBuffer<T> buffer_;

  std::atomic_bool need_stop_;
  std::mutex buff_lock_;
  std::condition_variable buff_is_not_full_condition_;
  std::condition_variable buff_is_not_empty_condition_;
};

}  // namespace locks

#endif  // LOCK_RING_BUFFER_H
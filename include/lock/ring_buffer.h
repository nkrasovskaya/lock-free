#ifndef LOCK_RING_BUFFER_H
#define LOCK_RING_BUFFER_H

#include <array>
#include <atomic>

namespace locks {

template <typename T, size_t buffer_size = 128>
class RingBuffer {
 public:
  RingBuffer() : head_idx_(0), tail_idx_(0), is_empty_(true) {}

  bool empty() const { return is_empty_; }

  bool full() const { return (tail_idx_ + 1) % buffer_size == head_idx_; }

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
      tail_idx_ = (tail_idx_ + 1) % buffer_size;
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
      head_idx_ = (head_idx_ + 1) % buffer_size;
    }
    return true;
  }

 private:
  std::atomic_size_t head_idx_;
  std::atomic_size_t tail_idx_;
  std::atomic_bool is_empty_;

  std::array<T, buffer_size> buffer_;
};

}  // namespace locks

#endif  // LOCK_RING_BUFFER_H
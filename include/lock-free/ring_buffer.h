#ifndef LOCK_FREE_RING_BUFFER_H
#define LOCK_FREE_RING_BUFFER_H

#include <array>
#include <atomic>

namespace lock_free {

template <typename T, size_t buffer_size = 128>
class RingBuffer {
 public:
  RingBuffer()
      : head_idx_(0),
        tail_idx_(0),
        used_(0),
        free_(buffer_size) {}

  size_t acquireWrite() {
    while (true) {
      auto old_tail = tail_idx_.load(std::memory_order_consume);
      while (free_.load(std::memory_order_consume) < 1) {
        // spin until success
      }

      size_t new_tail = (old_tail + 1) % buffer_size;
      free_--;
      if (tail_idx_.compare_exchange_strong(old_tail, new_tail)) {
        return old_tail;
      }
      free_.fetch_add(1, std::memory_order_relaxed);
    }
  }

    size_t tryAcquireWrite() {
    while (true) {
      auto old_tail = tail_idx_.load(std::memory_order_consume);
      if (free_.load(std::memory_order_consume) < 1) {
        return buffer_size + 1;
      }

      size_t new_tail = (old_tail + 1) % buffer_size;
      free_--;
      if (tail_idx_.compare_exchange_strong(old_tail, new_tail)) {
        return old_tail;
      }
      free_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  void releaseWrite() { used_.fetch_add(1, std::memory_order_release); }

  bool push(T&& data) {
    size_t widx = acquireWrite();
    buffer_[widx % buffer_size] = std::move(data);
    releaseWrite();
    return true;
  }

  bool tryPush(T&& data) {
    size_t widx = tryAcquireWrite();
    if (widx > buffer_size) {
      return false;
    }

    buffer_[widx % buffer_size] = std::move(data);
    releaseWrite();
    return true;
  }

  size_t acquireRead() {
    while (true) {
      auto old_head = head_idx_.load(std::memory_order_consume);
      while (used_.load(std::memory_order_consume) < 1) {
        // spin until success
      }

      size_t new_head = (old_head + 1) % buffer_size;
      used_--;
      if (head_idx_.compare_exchange_strong(old_head, new_head))
        return old_head;

      used_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  size_t tryAcquireRead() {
    while (true) {
      auto old_head = head_idx_.load(std::memory_order_consume);
      if (used_.load(std::memory_order_consume) < 1) {
        return buffer_size + 1;
      }

      size_t new_head = (old_head + 1) % buffer_size;
      used_--;
      if (head_idx_.compare_exchange_strong(old_head, new_head))
        return old_head;

      used_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  void releaseRead() { free_.fetch_add(1, std::memory_order_release); }

  bool pop(T& data) {
    size_t widx = acquireRead();

    data = std::move(buffer_[widx]);
    releaseRead();
    return true;
  }

  bool tryPop(T& data) {
    size_t widx = tryAcquireRead();
    if (widx > buffer_size) {
      return false;
    }

    data = std::move(buffer_[widx]);
    releaseRead();

    return true;
  }

 private:
  std::atomic_size_t head_idx_;
  std::atomic_size_t tail_idx_;

  std::atomic_int used_;
  std::atomic_int free_;

  std::array<T, buffer_size> buffer_;
};

}  // namespace lock_free

#endif  // LOCK_FREE_RING_BUFFER_H
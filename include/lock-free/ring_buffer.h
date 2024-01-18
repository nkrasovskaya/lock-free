#ifndef LOCK_FREE_RING_BUFFER_H
#define LOCK_FREE_RING_BUFFER_H

#include <atomic>
#include <vector>

namespace lock_free {

template <typename T>
class RingBuffer {
 public:
  RingBuffer(size_t buff_size)
      : head_idx_(0), tail_idx_(0), used_(0), free_(buff_size) {
    buffer_.resize(buff_size);
  }

  bool Enqueue(T&& data) {
    size_t widx = AcquireWrite();
    buffer_[widx % buffer_.size()] = std::move(data);
    ReleaseWrite();
    return true;
  }

  bool TryEnqueue(T&& data) {
    size_t widx = TryAcquireWrite();
    if (widx > buffer_.size()) {
      return false;
    }

    buffer_[widx % buffer_.size()] = std::move(data);
    ReleaseWrite();
    return true;
  }

  bool Dequeue(T& data) {
    size_t widx = AcquireRead();

    data = std::move(buffer_[widx]);
    ReleaseRead();
    return true;
  }

  bool TryDequeue(T& data) {
    size_t widx = TryAcquireRead();
    if (widx > buffer_.size()) {
      return false;
    }

    data = std::move(buffer_[widx]);
    ReleaseRead();

    return true;
  }

 private:
  std::atomic_size_t head_idx_;
  std::atomic_size_t tail_idx_;

  std::atomic_int used_;
  std::atomic_int free_;

  std::vector<T> buffer_;

  static_assert(std::atomic<size_t>::is_always_lock_free);
  static_assert(std::atomic<int>::is_always_lock_free);

    size_t AcquireWrite() {
    while (true) {
      auto old_tail = tail_idx_.load(std::memory_order_consume);
      while (free_.load(std::memory_order_consume) < 1) {
        // spin until success
      }

      size_t new_tail = (old_tail + 1) % buffer_.size();
      free_--;
      if (tail_idx_.compare_exchange_strong(old_tail, new_tail)) {
        return old_tail;
      }
      free_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  size_t TryAcquireWrite() {
    while (true) {
      auto old_tail = tail_idx_.load(std::memory_order_consume);
      if (free_.load(std::memory_order_consume) < 1) {
        return buffer_.size() + 1;
      }

      size_t new_tail = (old_tail + 1) % buffer_.size();
      free_--;
      if (tail_idx_.compare_exchange_strong(old_tail, new_tail)) {
        return old_tail;
      }
      free_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  void ReleaseWrite() { used_.fetch_add(1, std::memory_order_relaxed); }

  size_t AcquireRead() {
    while (true) {
      auto old_head = head_idx_.load(std::memory_order_consume);
      while (used_.load(std::memory_order_consume) < 1) {
        // spin until success
      }

      size_t new_head = (old_head + 1) % buffer_.size();
      used_--;
      if (head_idx_.compare_exchange_strong(old_head, new_head))
        return old_head;

      used_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  size_t TryAcquireRead() {
    while (true) {
      auto old_head = head_idx_.load(std::memory_order_consume);
      if (used_.load(std::memory_order_consume) < 1) {
        return buffer_.size() + 1;
      }

      size_t new_head = (old_head + 1) % buffer_.size();
      used_--;
      if (head_idx_.compare_exchange_strong(old_head, new_head))
        return old_head;

      used_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  void ReleaseRead() { free_.fetch_add(1, std::memory_order_relaxed); }
};

}  // namespace lock_free

#endif  // LOCK_FREE_RING_BUFFER_H
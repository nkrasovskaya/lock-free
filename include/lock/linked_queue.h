#ifndef LOCK_LINKED_QUEUE_H
#define LOCK_LINKED_QUEUE_H

#include <cassert>
#include <condition_variable>

namespace locks {

template <typename T>
struct QueueNode {
  QueueNode(T&& el) : data(std::move(el)), next(nullptr) {}
  T data;
  QueueNode *next;
};

template <typename T>
class LinkedQueue {
 public:
  LinkedQueue() : head_(nullptr), tail_(nullptr) {}

  bool Empty() const { return head_ == nullptr && tail_ == nullptr; }

  bool Enqueue(T&& data) {
    if (Empty()) {
      head_ = new QueueNode(std::move(data));
      tail_ = head_;
    } else {
      if (tail_ == nullptr) {
        // impossible
        assert(false);
        return false;
      }
      assert(tail_->next == nullptr);
      tail_->next = new QueueNode(std::move(data));
      tail_ = tail_->next;
    }

    return true;
  }

  bool Dequeue(T& data) {
    if (Empty()) {
      return false;
    }
    assert(head_ != nullptr && tail_ != nullptr);
    data = std::move(head_->data);
    QueueNode<T> *tmp = head_;
    head_ = head_->next;
    if (head_ == nullptr) {
      // last element
      tail_ = nullptr;
    }

    delete tmp;

    return true;
  }

 private:
  QueueNode<T> *head_;
  QueueNode<T> *tail_;
};

template <typename T>
class LinkedQueueThreadSafe {
 public:
  LinkedQueueThreadSafe()
      : lqueue_(), need_stop_(false) {}

  ~LinkedQueueThreadSafe() {}

  bool Enqueue(T&& data) {
    std::unique_lock<std::mutex> lock(buff_lock_);
    if (need_stop_) {
      return false;
    }

    bool res = lqueue_.Enqueue(std::move(data));
    assert(res);

    buff_is_not_empty_condition_.notify_one();

    return res;
  }

  bool Dequeue(T& data) {
    std::unique_lock<std::mutex> lock(buff_lock_);
    buff_is_not_empty_condition_.wait(lock, [this] {
      return std::forward<bool>(need_stop_) || !lqueue_.Empty();
    });
    if (need_stop_ && lqueue_.Empty()) {
      return false;
    }

    bool res = lqueue_.Dequeue(data);
    assert(res);

    return res;
  }

  void Stop() {
    need_stop_ = true;
    buff_is_not_empty_condition_.notify_all();
  }

 private:
  LinkedQueue<T> lqueue_;

  std::atomic_bool need_stop_;
  std::mutex buff_lock_;
  std::condition_variable buff_is_not_empty_condition_;
};

}  // namespace locks

#endif  // LOCK_LINKED_QUEUE_H
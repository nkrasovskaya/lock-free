#ifndef LOCK_FREE_LINKED_QUEUE_H
#define LOCK_FREE_LINKED_QUEUE_H

#include <atomic>
#include <cassert>
#include <cstring>
#include <map>
#include <set>
#include <thread>

#define MAX_THREAD_NUM 128
#define HP_PER_THREAD 2

namespace lock_free {

static int ccount = 0;

template <typename T>
class HazardPointers {
 public:
  HazardPointers(size_t tnum)
      : max_count_(2 * HP_PER_THREAD * tnum),
        client_threads_num_(tnum),
        scan_is_run_(false) {
    memset(hp_arr_, 0, MAX_THREAD_NUM * HP_PER_THREAD * sizeof(T*));
  }

  ~HazardPointers() {
    Scan();
  }

  void AddThread(std::thread::id tid) {
    hp_map_.insert({tid, tcount_++});
  }

  void AcquireHazardPointer(std::thread::id tid, size_t ind, T* node) {
    hp_arr_[hp_map_[tid]][ind] = node;
  }

  void ReleaseHazardPointer(std::thread::id tid, size_t ind) {
    hp_arr_[hp_map_[tid]][ind] = nullptr;
  }

  void UnrefNode(T* node) {
    while (unref_count_ >= max_count_ || scan_is_run_) {
      ;
    }
    unref_nodes_[unref_count_++] = node;
    if (unref_count_ >= max_count_) {
      Scan();
    }
  }

 private:
  size_t client_threads_num_;
  size_t max_count_;
  T* hp_arr_[MAX_THREAD_NUM][HP_PER_THREAD];
  std::map<std::thread::id, size_t> hp_map_;
  T* unref_nodes_[4 * HP_PER_THREAD * MAX_THREAD_NUM];
  std::atomic_size_t unref_count_ = 0;

  std::atomic_bool scan_is_run_;

  std::atomic_size_t tcount_ = 0;

  void Scan() {
    bool is_run = false;
    if (!scan_is_run_.compare_exchange_strong(is_run, true,
                                              std::memory_order_relaxed)) {
      return;
    }

    std::set<T*> plist;
    for (auto& p : hp_map_) {
      for (int i = 0; i < HP_PER_THREAD; ++i) {
        if (hp_arr_[p.second][i] != nullptr) {
          plist.insert(hp_arr_[p.second][i]);
        }
      }
    }

    size_t new_unref_count = 0;
    T* new_unref_nodes[max_count_ * 2];
    for (int i = 0; i < unref_count_; ++i) {
      if (plist.contains(unref_nodes_[i])) {
        new_unref_nodes[new_unref_count++] = unref_nodes_[i];
      } else {
        delete unref_nodes_[i];
      }
    }

    for (int i = 0; i < new_unref_count; ++i) {
      unref_nodes_[i] = new_unref_nodes[i];
    }

    unref_count_ = new_unref_count;
    scan_is_run_.store(false, std::memory_order_release);
  }
};

template <typename T>
struct QueueNode {
  QueueNode() : next(nullptr) {}
  QueueNode(T&& el) : data(std::move(el)), next(nullptr) {}
  T data;
  std::atomic<QueueNode*> next;
};

template <typename T>
class LinkedQueue {
 public:
  LinkedQueue(size_t tnum) : hp_(tnum) {
    QueueNode<T>* node = new QueueNode<T>;
    head_.store(node, std::memory_order_release);
    tail_.store(node, std::memory_order_release);
  }

  ~LinkedQueue() {
    QueueNode<T>* head = head_.load(std::memory_order_relaxed);
    QueueNode<T>* tail = tail_.load(std::memory_order_relaxed);

    assert(head == tail);
    assert(head->next == nullptr);

    delete head_;
  }

  void RegisterThread() { hp_.AddThread(std::this_thread::get_id()); }

  bool Enqueue(T&& data) {
    assert(data != nullptr);
    QueueNode<T>* node = new QueueNode<T>(std::move(data));
    assert(node->data != nullptr && "xdvxdv");

    QueueNode<T>* t = nullptr;
    while (true) {
      t = tail_.load(std::memory_order_relaxed);
      hp_.AcquireHazardPointer(std::this_thread::get_id(), 0, t);
      if (t != tail_.load(std::memory_order_acquire)) {
        continue;
      }

      QueueNode<T>* next = t->next.load(std::memory_order_acquire);
      if (t != tail_) {
        continue;
      }

      if (next != nullptr) {
        tail_.compare_exchange_weak(t, next, std::memory_order_release);
        continue;
      }
      QueueNode<T>* tmp = nullptr;
      if (t->next.compare_exchange_strong(tmp, node,
                                          std::memory_order_release)) {
        break;
      }
    }

    tail_.compare_exchange_strong(t, node, std::memory_order_acq_rel);
    hp_.ReleaseHazardPointer(std::this_thread::get_id(), 0);

    return true;
  }

  bool TryDequeue(T& data) {
    QueueNode<T>* head;
    QueueNode<T>* next;
    T* res;
    while (true) {
      head = head_.load(std::memory_order_relaxed);
      hp_.AcquireHazardPointer(std::this_thread::get_id(), 0, head);

      if (head != head_.load(std::memory_order_acquire)) {
        continue;
      }

      QueueNode<T>* tail = tail_.load(std::memory_order_relaxed);
      next = head->next.load(std::memory_order_acquire);

      hp_.AcquireHazardPointer(std::this_thread::get_id(), 1, next);

      if (head != head_.load(std::memory_order_relaxed)) {
        continue;
      }

      if (next == nullptr) {
        // empty
        hp_.ReleaseHazardPointer(std::this_thread::get_id(), 0);
        return false;
      }

      if (head == tail) {
        tail_.compare_exchange_strong(tail, next, std::memory_order_release);
        continue;
      }

      res = &(next->data);
      if (head_.compare_exchange_strong(head, next,
                                        std::memory_order_release)) {
        break;
      }
    }
    data = std::move(*res);

    hp_.ReleaseHazardPointer(std::this_thread::get_id(), 0);
    hp_.ReleaseHazardPointer(std::this_thread::get_id(), 1);

    hp_.UnrefNode(head);

    return true;
  }

 private:
  std::atomic<QueueNode<T>*> head_;
  std::atomic<QueueNode<T>*> tail_;

  HazardPointers<QueueNode<T>> hp_;
};

}  // namespace lock_free

#endif  // LOCK_FREE_LINKED_QUEUE_H
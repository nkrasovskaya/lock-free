#ifndef TASKS_QUEUE_H
#define TASKS_QUEUE_H

#include <functional>

class LogMessage;

#ifdef LOCK_FREE
#include "lock-free/ring_buffer.h"

typedef lock_free::RingBuffer<std::function<void()>> TasksQueue;

typedef lock_free::RingBuffer<std::unique_ptr<LogMessage>> LoggerQueue;
#else  // LOCK_FREE
#include "lock/ring_buffer.h"

typedef locks::RingBufferThreadSafe<std::function<void()>> TasksQueue;

typedef locks::RingBufferThreadSafe<std::unique_ptr<LogMessage>> LoggerQueue;
#endif  // LOCK_FREE

#endif  // IQUEUE_H
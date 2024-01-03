cmake_minimum_required(VERSION 3.20.0)
project(LockFree VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_C_COMPILER /usr/bin/gcc-12)
set(CMAKE_CXX_COMPILER /usr/bin/g++-12)

include_directories(include/)

set(SOURCES_lock-thread-pool
  src/lock/logger.cpp
  src/lock/main.cpp
  src/lock/task_generator.cpp
  src/lock/thread_pool.cpp)

add_executable(lock-thread-pool ${SOURCES_lock-thread-pool})

set(SOURCES_lock-free-thread-pool
src/lock-free/logger.cpp
src/lock-free/main.cpp
src/lock-free/task_generator.cpp
src/lock-free/thread_pool.cpp)
add_executable(lock-free-thread-pool ${SOURCES_lock-free-thread-pool})

enable_testing()
add_subdirectory(tests)


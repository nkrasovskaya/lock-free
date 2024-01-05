cmake_minimum_required(VERSION 3.20.0)
project(LockFree VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_C_COMPILER /usr/bin/gcc-12)
set(CMAKE_CXX_COMPILER /usr/bin/g++-12)

include_directories(include/ 3rd_party/include/)

set(SOURCES
  src/config.cpp
  src/logger.cpp
  src/main.cpp
  src/task_generator.cpp
  src/thread_pool.cpp)

add_executable(lock-thread-pool ${SOURCES})

add_executable(lock-free-thread-pool ${SOURCES})

target_compile_definitions(lock-free-thread-pool PUBLIC LOCK_FREE)

enable_testing()
add_subdirectory(tests)


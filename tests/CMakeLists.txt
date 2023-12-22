find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS}
                    "${PROJECT_SOURCE_DIR}/include")

file(GLOB TEST_SOURCES
  *.cpp
)

include(GoogleTest)

add_executable(ring-buffer-test
  ${TEST_SOURCES}
)

target_link_libraries(ring-buffer-test
  ${GTEST_BOTH_LIBRARIES}
)

gtest_add_tests(ring-buffer-test "" AUTO)
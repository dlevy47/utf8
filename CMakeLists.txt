cmake_minimum_required(VERSION 3.10)
project(utf8)
enable_testing()

find_package(GTest CONFIG REQUIRED)

add_library(utf8 INTERFACE)
target_include_directories(utf8 INTERFACE "lib")

add_executable(tests tests/test.cc)
target_link_libraries(tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main utf8)

add_test(NAME tests COMMAND tests)

install(
  TARGETS utf8
  EXPORT utf8Targets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

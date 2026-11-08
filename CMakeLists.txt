cmake_minimum_required(VERSION 3.23)
project(utf8)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/WX)
endif()

find_package(GTest CONFIG REQUIRED)

add_library(utf8 INTERFACE)
target_include_directories(
  utf8 INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
file(GLOB_RECURSE SOURCES "lib/*.hh")
target_sources(utf8 INTERFACE FILE_SET headers BASE_DIRS "lib/" TYPE HEADERS FILES ${SOURCES})

add_executable(
  tests
  tests/test.cc
  tests/scanner_test.cc
  tests/string_test.cc
  tests/utf8_test.cc
)
target_link_libraries(tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main utf8)
target_include_directories(
  tests PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/lib
)
add_test(NAME tests COMMAND tests)

install(EXPORT utf8Targets FILE utf8Config.cmake NAMESPACE utf8:: DESTINATION share/utf8)

install(
  TARGETS utf8
  EXPORT utf8Targets
  FILE_SET headers
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

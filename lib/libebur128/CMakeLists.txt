cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(libebur128 C)

option(BUILD_SHARED_LIBS
    "Build shared libraries instead of static ones" ON)

if(ENABLE_FUZZER)
  enable_language(CXX)
  set(SANITIZER_FLAGS "-fsanitize=address,undefined,unsigned-integer-overflow")
  set(FUZZER_FLAGS "${SANITIZER_FLAGS},fuzzer")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(utils)
include(GNUInstallDirs)

add_subdirectory(ebur128)
add_subdirectory(test)

to_yes_no(SUMMARY_HAS_QUEUE)

if(ENABLE_INTERNAL_QUEUE_H)
  set(USE_QUEUE "using own copy of queue.h")
else()
  set(USE_QUEUE "using system copy of queue.h")
endif()

##### Print status
message(STATUS "Status          found / disabled --")
message(STATUS "queue.h:        ${SUMMARY_HAS_QUEUE}"        "     ${USE_QUEUE}")

if(BUILD_SHARED_LIBS)
  message(STATUS "Building shared library (set BUILD_SHARED_LIBS to OFF to build static)")
else()
  message(STATUS "Building static library")
endif()

if(ENABLE_TESTS)
  message(STATUS "building tests!")
else()
  message(STATUS "not building tests, set ENABLE_TESTS to ON to enable")
endif()

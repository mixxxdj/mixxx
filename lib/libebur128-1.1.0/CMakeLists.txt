cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(libebur128 C)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include(utils)
include(GNUInstallDirs)

add_subdirectory(ebur128)
add_subdirectory(test)

to_yes_no(SUMMARY_HAS_QUEUE SUMMARY_SPEEXDSP_FOUND)
to_yes_no(DISABLE_SPEEXDSP)

if(ENABLE_INTERNAL_QUEUE_H)
  set(USE_QUEUE "using own copy of queue.h")
else()
  set(USE_QUEUE "using system copy of queue.h")
endif()

##### Print status
message(STATUS "Status          found / disabled --")
message(STATUS "queue.h:        ${SUMMARY_HAS_QUEUE}"        "     ${USE_QUEUE}")
message(STATUS "speexdsp:       ${SUMMARY_SPEEXDSP_FOUND}"   "     ${DISABLE_SPEEXDSP}")

if(BUILD_STATIC_LIBS)
  message(STATUS "build static library and shared library!")
else()
  message(STATUS "not building static library, set BUILD_STATIC_LIBS to ON to enable")
endif()

if(NOT SUMMARY_HAS_QUEUE AND NOT ENABLE_INTERNAL_QUEUE_H)
  message(FATAL_ERROR "queue.h not found, please set ENABLE_INTERNAL_QUEUE_H to ON")
endif()

if(ENABLE_TESTS)
  message(STATUS "building tests!")
else()
  message(STATUS "not building tests, set ENABLE_TESTS to ON to enable")
endif()

cmake_minimum_required(VERSION 3.22.1)
project(libremidi-example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find libremidi in the parent directory
set(LIBREMIDI_NO_ANDROID 0)
set(BUILD_SHARED_LIBS 1)
set(LIBREMIDI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../..")
add_subdirectory(${LIBREMIDI_ROOT} libremidi-build)

# Create shared library
add_library(libremidi-example SHARED
    native-lib.cpp
)

# Link libremidi
target_link_libraries(libremidi-example
    libremidi
    android
    log
)

# Include directories
target_include_directories(libremidi-example PRIVATE
    ${LIBREMIDI_ROOT}/include
)
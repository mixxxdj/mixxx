cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(pylibremidi)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL 1)

set(LIBREMIDI_PYTHON 1)
set(LIBREMIDI_FIND_BOOST 1)
set(LIBREMIDI_HEADER_ONLY 1)
set(LIBREMIDI_NEEDS_READERWRITERQUEUE 1)

set(BOOST_EXCLUDE_LIBRARIES context coroutine)

add_subdirectory(../.. libremidi-src)

find_package(Python 3.10 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(Python 3.10 COMPONENTS Development.SABIModule QUIET)

if(NOT readerwriterqueue_FOUND AND NOT TARGET readerwriterqueue)
    FetchContent_Declare(
            readerwriterqueue
            GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue
            GIT_TAG        master
    )

    FetchContent_MakeAvailable(readerwriterqueue)
endif()

FetchContent_Declare(
    nanobind
    GIT_REPOSITORY https://github.com/wjakob/nanobind
    GIT_TAG        master
)

FetchContent_MakeAvailable(nanobind)

if(Python_VERSION VERSION_GREATER_EQUAL "3.12" AND Python_SABIModule_FOUND)
    message(STATUS "Building with Python stable ABI (abi3)")
    nanobind_add_module(pylibremidi STABLE_ABI NB_STATIC pylibremidi.cpp)
else()
    message(STATUS "Building with Python version-specific ABI")
    nanobind_add_module(pylibremidi NB_STATIC pylibremidi.cpp)
endif()
target_link_libraries(pylibremidi PUBLIC libremidi readerwriterqueue Boost::headers Boost::variant2 Boost::container)

install(TARGETS pylibremidi LIBRARY DESTINATION .)

set(CAN_RUN_STUB_GENERATOR TRUE)
if(APPLE)
    execute_process(COMMAND uname -m OUTPUT_VARIABLE HOST_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(CMAKE_OSX_ARCHITECTURES)
        set(TARGET_ARCH "${CMAKE_OSX_ARCHITECTURES}")
    else()
        set(TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    if(NOT HOST_ARCH STREQUAL TARGET_ARCH)
        set(CAN_RUN_STUB_GENERATOR FALSE)
    endif()
elseif(WIN32)
    if(CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
        set(CAN_RUN_STUB_GENERATOR FALSE)
    endif()
endif()

if(CAN_RUN_STUB_GENERATOR)
    nanobind_add_stub(
        pylibremidi_stub
        MODULE pylibremidi
        OUTPUT pylibremidi.pyi
        PYTHON_PATH $<TARGET_FILE_DIR:pylibremidi>
        DEPENDS pylibremidi
    )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/pylibremidi.pyi
        DESTINATION .
        OPTIONAL
    )
endif()

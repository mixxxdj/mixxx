### C++ features ###
check_cxx_source_compiles("#include <thread>\nint main() { std::jthread t; }" LIBREMIDI_HAS_STD_JTHREAD)
check_cxx_source_compiles("#include <flat_set>\nint main() { std::flat_set<int> t; }" LIBREMIDI_HAS_STD_FLAT_SET)
check_cxx_source_compiles("#include <print>\nint main() { std::println(\"foo: {}\", 123); }" LIBREMIDI_HAS_STD_PRINTLN)
check_cxx_source_compiles("#include <semaphore>\nint main() { std::binary_semaphore t{0}; }" LIBREMIDI_HAS_STD_SEMAPHORE)
check_cxx_source_compiles("#include <stop_token>\n#include <thread>\nint main() { std::jthread t; }" LIBREMIDI_HAS_STD_STOP_TOKEN)

if(NOT WIN32)
  check_include_file_cxx("sys/eventfd.h" LIBREMIDI_HAS_EVENTFD)
  check_include_file_cxx("sys/timerfd.h" LIBREMIDI_HAS_TIMERFD)
endif()

check_cxx_compiler_flag(-Werror=return-type LIBREMIDI_CXX_HAS_WERROR_RETURN_TYPE)
check_cxx_compiler_flag(-Wno-gnu-statement-expression-from-macro-expansion LIBREMIDI_CXX_HAS_WNO_GNU_STATEMENT)
check_cxx_compiler_flag(-Wno-c99-extensions LIBREMIDI_CXX_HAS_WNO_C99_EXTENSIONS)

### Dependencies ###
find_package(Threads)

# ni-midi2
if(LIBREMIDI_NI_MIDI2 AND NOT TARGET ni::midi2)
  FetchContent_Declare(
      ni-midi2
      GIT_REPOSITORY https://github.com/midi2-dev/ni-midi2
      GIT_TAG        main
  )

  FetchContent_MakeAvailable(ni-midi2)
endif()

# boost
if(LIBREMIDI_NO_BOOST AND LIBREMIDI_FIND_BOOST)
  message(FATAL_ERROR "LIBREMIDI_NO_BOOST and LIBREMIDI_FIND_BOOST are incompatible")
endif()

if(LIBREMIDI_FIND_BOOST)
  set(BOOST_INCLUDE_LIBRARIES headers container)
  if(NOT LIBREMIDI_NO_NETWORK)
    list(APPEND BOOST_INCLUDE_LIBRARIES asio)
  endif()
  if(LIBREMIDI_EXAMPLES)
    list(APPEND BOOST_INCLUDE_LIBRARIES cobalt)
  endif()
  if(LIBREMIDI_PYTHON)
    list(APPEND BOOST_INCLUDE_LIBRARIES variant2)
  endif()
  find_package(Boost 1.90 OPTIONAL_COMPONENTS ${BOOST_INCLUDE_LIBRARIES})

  if(NOT Boost_FOUND)
    set(BOOST_ENABLE_CMAKE ON)

    FetchContent_Declare(
            Boost
            OVERRIDE_FIND_PACKAGE TRUE
            URL "https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.tar.xz"
    )

    FetchContent_MakeAvailable(Boost)

    find_package(Boost REQUIRED OPTIONAL_COMPONENTS cobalt)
  endif()
endif()

# readerwriterqueue
if(UNIX AND NOT APPLE AND NOT LIBREMIDI_NO_PIPEWIRE)
  set(LIBREMIDI_NEEDS_READERWRITERQUEUE 1)
endif()
if(LIBREMIDI_NEEDS_READERWRITERQUEUE AND NOT TARGET readerwriterqueue)
  find_package(readerwriterqueue)

  if(NOT readerwriterqueue_FOUND)
    FetchContent_Declare(
        rwq
        GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue
        GIT_TAG        master
    )

    FetchContent_MakeAvailable(rwq)
  endif()
endif()

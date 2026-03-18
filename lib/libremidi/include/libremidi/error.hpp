#pragma once
#include <libremidi/config.hpp>

#if !defined(_MSC_VER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <libremidi/system_error2.hpp>
#if !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif

#include <functional>
#include <string_view>
#include <version>

#if __has_include(<source_location>) && (__cpp_lib_source_location >= 201907L)
  #include <source_location>
NAMESPACE_LIBREMIDI
{
using source_location = std::source_location;
}
#else
NAMESPACE_LIBREMIDI
{
struct source_location
{
  static source_location current() noexcept { return {}; }
  int line() const noexcept { return -1; }
  int offset() const noexcept { return -1; }
  const char* function_name() const noexcept { return "(unknown)"; }
  const char* file_name() const noexcept { return "(unknown)"; }
};
}
#endif

NAMESPACE_LIBREMIDI
{
inline auto from_errc(int64_t ret) noexcept
{
  return static_cast<std::errc>(-ret);
}

/*! \brief Error callback function
    \param type Type of error.
    \param errorText Error description.

    Note that class behaviour is undefined after a critical error (not
    a warning) is reported.
 */
using midi_error_callback
    = std::function<void(std::string_view errorText, const source_location&)>;
using midi_warning_callback
    = std::function<void(std::string_view errorText, const source_location&)>;
}

#if !defined(LIBREMIDI_LOG)
  #if !defined(__LIBREMIDI_DEBUG__)
    #define LIBREMIDI_LOG(...) \
      do                       \
      {                        \
      } while (0)
  #else
    #include <iostream>
    #define LIBREMIDI_LOG(...)        \
      do                              \
      {                               \
        [](auto&&... args) {          \
          (std::cerr << ... << args); \
          std::cerr << std::endl;     \
        }(__VA_ARGS__);               \
      } while (0)
  #endif
#endif

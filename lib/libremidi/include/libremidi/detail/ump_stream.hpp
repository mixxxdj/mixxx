#pragma once
#include <libremidi/cmidi2.hpp>
#include <libremidi/error.hpp>

#include <cstdint>

NAMESPACE_LIBREMIDI
{
enum class segmentation_error
{
  no_error,
  need_space,
  other
};

/**
 * Utility function to segment an ump stream into individual messages.
 * Used to send a stream to APIs that work message-by-message.
 */
inline stdx::error
segment_ump_stream(const uint32_t* ump_stream, int64_t count, auto write_func, auto realloc_func)
{
  while (count > 0)
  {
    // Handle NOOP (or padding)
    while (count > 0 && ump_stream[0] == 0)
    {
      count--;
      ump_stream++;
    }

    if (count == 0)
      break;

    const auto ump_bytes = cmidi2_ump_get_num_bytes(ump_stream[0]);

    // FIXME std::expected, propagate the error back to caller?
    switch (int err = static_cast<int>(write_func(ump_stream, ump_bytes)))
    {
      case 0:
        break;
      case -ENOMEM:
      case ENOMEM:
        // Try again if we didn't have enough space in the OS queue
        realloc_func();
        if (auto err = write_func(ump_stream, ump_bytes); err != std::errc{})
          return std::make_error_code(err);
        break;
      default:
        return from_errc(err);
    }

    const auto ump_uints = ump_bytes / 4;
    ump_stream += ump_uints;
    count -= ump_uints;
  }

  return stdx::error{};
}

}

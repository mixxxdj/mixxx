#pragma once
#include <libremidi/backends/alsa_raw/config.hpp>

#include <chrono>
#include <functional>

NAMESPACE_LIBREMIDI::alsa_raw_ump
{
struct input_configuration
{
  std::function<bool(const manual_poll_parameters&)> manual_poll;
  std::chrono::milliseconds poll_period{2};
};

struct output_configuration
{
  /**
   * For large messages, chunk their content and wait.
   * Setting a null optional will disable chunking.
   */
  std::optional<chunking_parameters> chunking;
};

struct observer_configuration : public alsa_raw_observer_configuration
{
};
}

#pragma once
#include <libremidi/backends/jack/config.hpp>

NAMESPACE_LIBREMIDI::jack_ump
{
struct input_configuration
{
  std::string client_name = "libremidi client";

  jack_client_t* context{};
  std::function<void(libremidi::jack_callback)> set_process_func{};
  std::function<void(int64_t)> clear_process_func{};
};

struct output_configuration
{
  std::string client_name = "libremidi client";

  jack_client_t* context{};
  std::function<void(libremidi::jack_callback)> set_process_func{};
  std::function<void(int64_t)> clear_process_func{};

  int32_t ringbuffer_size = 16384;
  bool direct = false;
};

struct observer_configuration
{
  std::string client_name = "libremidi client";
  jack_client_t* context{};
};

}

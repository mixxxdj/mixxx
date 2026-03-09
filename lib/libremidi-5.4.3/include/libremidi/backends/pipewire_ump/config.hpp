#pragma once
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/config.hpp>

NAMESPACE_LIBREMIDI::pipewire_ump
{
struct input_configuration
{
  std::string client_name = "libremidi client";

  pw_main_loop* context{};
  pw_filter* filter{};
  std::function<void(libremidi::pipewire_callback)> set_process_func{};
  std::function<void(int64_t)> clear_process_func{};
};

struct output_configuration
{
  std::string client_name = "libremidi client";

  pw_main_loop* context{};
  pw_filter* filter{};
  std::function<void(libremidi::pipewire_callback)> set_process_func{};
  std::function<void(int64_t)> clear_process_func{};

  int64_t output_buffer_size{65536};
};

struct observer_configuration
{
  std::string client_name = "libremidi client";

  pw_main_loop* context{};
};

}

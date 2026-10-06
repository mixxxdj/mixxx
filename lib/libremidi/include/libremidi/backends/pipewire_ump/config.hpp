#pragma once
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/config.hpp>

NAMESPACE_LIBREMIDI::pipewire_ump
{
// Same borrow-mode contract as pipewire_input_configuration.
struct input_configuration
{
  std::string client_name = "libremidi client";

  pw_thread_loop* thread_loop{};
  pw_main_loop* main_loop{};
  pw_core* core{};
};

struct output_configuration
{
  std::string client_name = "libremidi client";

  pw_thread_loop* thread_loop{};
  pw_main_loop* main_loop{};
  pw_core* core{};

  int64_t output_buffer_size{65536};
};

struct observer_configuration
{
  std::string client_name = "libremidi client";

  pw_thread_loop* thread_loop{};
  pw_main_loop* main_loop{};
  pw_core* core{};
};
}

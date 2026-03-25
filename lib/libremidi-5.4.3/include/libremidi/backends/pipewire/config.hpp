#pragma once
#include <libremidi/config.hpp>

#include <cinttypes>
#include <cstdint>
#include <functional>
#include <string>

extern "C" {
struct pw_main_loop;
struct pw_filter;
struct spa_io_position;
}

NAMESPACE_LIBREMIDI
{
using pipewire_callback_function = std::function<void(spa_io_position*)>;
struct pipewire_callback
{
  int64_t token;
  pipewire_callback_function callback;
};

struct pipewire_input_configuration
{
  std::string client_name = "libremidi client";

  pw_main_loop* context{};
  pw_filter* filter{};
  std::function<void(pipewire_callback)> set_process_func{};
  std::function<void(int64_t)> clear_process_func{};
};

struct pipewire_output_configuration
{
  std::string client_name = "libremidi client";

  pw_main_loop* context{};
  pw_filter* filter{};
  std::function<void(pipewire_callback)> set_process_func{};
  std::function<void(int64_t)> clear_process_func{};

  int64_t output_buffer_size{65536};
};

struct pipewire_observer_configuration
{
  std::string client_name = "libremidi client";

  pw_main_loop* context{};
};

}

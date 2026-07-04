#pragma once
#include <libremidi/config.hpp>

#include <cstdint>
#include <string>

extern "C" {
struct pw_thread_loop;
struct pw_main_loop;
struct pw_core;
}

NAMESPACE_LIBREMIDI
{
// To embed in a host that owns the pipewire setup, supply `core` plus
// one of `thread_loop` / `main_loop` — libremidi adopts them without
// taking ownership. All-null falls back to the shared singleton.
struct pipewire_input_configuration
{
  std::string client_name = "libremidi client";

  pw_thread_loop* thread_loop{};
  pw_main_loop* main_loop{};
  pw_core* core{};
};

struct pipewire_output_configuration
{
  std::string client_name = "libremidi client";

  pw_thread_loop* thread_loop{};
  pw_main_loop* main_loop{};
  pw_core* core{};

  int64_t output_buffer_size{65536};
};

struct pipewire_observer_configuration
{
  std::string client_name = "libremidi client";

  pw_thread_loop* thread_loop{};
  pw_main_loop* main_loop{};
  pw_core* core{};
};
}

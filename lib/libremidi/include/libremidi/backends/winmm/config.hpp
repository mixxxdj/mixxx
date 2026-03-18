#pragma once
#include <libremidi/config.hpp>

#include <chrono>
#include <functional>

NAMESPACE_LIBREMIDI
{

struct winmm_input_configuration
{
  int sysex_buffer_size = 1024;
  int sysex_buffer_count = 4;
};

struct winmm_output_configuration
{
};

struct poll_parameters
{
  std::function<void()> callback;
};

struct winmm_observer_configuration
{
  std::chrono::milliseconds poll_period{100};
  std::function<void(const poll_parameters&)> manual_poll;
};

}

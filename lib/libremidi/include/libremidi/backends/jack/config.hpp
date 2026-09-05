#pragma once
#include <libremidi/config.hpp>

#include <cstdint>
#include <functional>
#include <string>

extern "C" {
typedef struct _jack_client jack_client_t;
typedef uint32_t jack_nframes_t;
typedef int (*JackProcessCallback)(jack_nframes_t nframes, void* arg);
}

NAMESPACE_LIBREMIDI
{
using jack_callback_function = std::function<void(int nframes)>;
struct jack_callback
{
  int64_t token;
  std::function<void(int nframes)> callback;
};
struct jack_input_configuration
{
  std::string client_name = "libremidi client";

  jack_client_t* context{};
  std::function<void(jack_callback)> set_process_func{};
  std::function<void(int64_t)> clear_process_func{};
};

struct jack_output_configuration
{
  std::string client_name = "libremidi client";

  jack_client_t* context{};
  std::function<void(jack_callback)> set_process_func{};
  std::function<void(int64_t)> clear_process_func{};

  int32_t ringbuffer_size = 16384;
  bool direct = false;
};

struct jack_observer_configuration
{
  std::string client_name = "libremidi client";
  jack_client_t* context{};
};

}

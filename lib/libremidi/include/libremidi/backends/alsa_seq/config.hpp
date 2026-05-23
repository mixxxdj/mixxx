#pragma once
#include <libremidi/backends/alsa_raw/config.hpp>

#if __has_include(<alsa/asoundlib.h>)
  #include <alsa/asoundlib.h>
#else
extern "C" {
typedef struct _snd_seq snd_seq_t;
typedef struct snd_seq_event snd_seq_event_t;
typedef struct snd_seq_addr
{
  unsigned char client;
  unsigned char port;
} snd_seq_addr_t;
}
#endif

NAMESPACE_LIBREMIDI::alsa_seq
{

// Possible parameters:
// - Direct / non-direct output
// - Timer source used (high resolution, etc)
// - Timestamping
// - Tempo, ppq?

struct poll_parameters
{
  snd_seq_addr_t addr{};
  std::function<int(const snd_seq_event_t&)> callback;
};

struct input_configuration
{
  using poll_parameters_type = poll_parameters;

  std::string client_name = "libremidi client";
  snd_seq_t* context{};
  std::function<bool(const poll_parameters&)> manual_poll;
  std::function<bool(snd_seq_addr_t)> stop_poll;
  std::chrono::milliseconds poll_period{2};

  static constexpr int midi_version = 1;
};

struct output_configuration
{
  std::string client_name = "libremidi client";
  snd_seq_t* context{};

  static constexpr int midi_version = 1;
};

struct observer_configuration
{
  std::string client_name = "libremidi client";
  snd_seq_t* context{};
  std::function<bool(const poll_parameters&)> manual_poll;
  std::function<bool(snd_seq_addr_t)> stop_poll;
  std::chrono::milliseconds poll_period{100};

  static constexpr int midi_version = 1;
};

}

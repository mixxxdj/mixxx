#pragma once
#include <libremidi/backends/alsa_seq/config.hpp>

#if __has_include(<alsa/asoundlib.h>)
  #include <alsa/asoundlib.h>
#endif

// Necessary for the versions of ALSA that did have UMP
// through snd_seq_event, before snd_seq_ump_event existed.
extern "C" {
typedef struct snd_seq_ump_event snd_seq_ump_event_t;
}

NAMESPACE_LIBREMIDI::alsa_seq_ump
{

struct poll_parameters
{
  snd_seq_addr_t addr{};
  std::function<int(const snd_seq_ump_event_t&)> callback;
};

struct input_configuration
{
  using poll_parameters_type = poll_parameters;

  std::string client_name = "libremidi client";
  snd_seq_t* context{};
  std::function<bool(const poll_parameters&)> manual_poll;
  std::function<bool(snd_seq_addr_t)> stop_poll;
  std::chrono::milliseconds poll_period{2};

  static constexpr int midi_version = 2;
};

struct output_configuration
{
  std::string client_name = "libremidi client";
  snd_seq_t* context{};

  static constexpr int midi_version = 2;
};

struct observer_configuration
{
  std::string client_name = "libremidi client";
  snd_seq_t* context{};
  std::function<bool(const libremidi::alsa_seq::poll_parameters&)> manual_poll;
  std::function<bool(snd_seq_addr_t)> stop_poll;
  std::chrono::milliseconds poll_period{100};

  static constexpr int midi_version = 2;
};

}

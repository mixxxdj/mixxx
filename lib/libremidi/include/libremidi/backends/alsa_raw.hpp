#pragma once
#include <libremidi/backends/alsa_raw/midi_in.hpp>
#include <libremidi/backends/alsa_raw/midi_out.hpp>
#include <libremidi/backends/alsa_raw/observer.hpp>

#include <string_view>
// Credits: greatly inspired from
// https://ccrma.stanford.edu/~craig/articles/linuxmidi/alsa-1.0/alsarawmidiout.c
// https://ccrma.stanford.edu/~craig/articles/linuxmidi/alsa-1.0/alsarawportlist.c
// Thanks Craig Stuart Sapp <craig@ccrma.stanford.edu>

NAMESPACE_LIBREMIDI::alsa_raw
{
struct backend
{
  using midi_in = midi_in_impl;
  using midi_out = midi_out_impl;
  using midi_observer = observer_impl;
  using midi_in_configuration = alsa_raw_input_configuration;
  using midi_out_configuration = alsa_raw_output_configuration;
  using midi_observer_configuration = alsa_raw_observer_configuration;
  static const constexpr auto API = libremidi::API::ALSA_RAW;
  static const constexpr std::string_view name = "alsa_raw";
  static const constexpr std::string_view display_name = "ALSA (raw)";

  static inline bool available() noexcept
  {
    static const libasound& snd = libasound::instance();
    return snd.available && snd.rawmidi.available;
  }
};
}

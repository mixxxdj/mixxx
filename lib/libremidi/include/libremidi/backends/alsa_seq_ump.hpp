#pragma once

#include <libremidi/backends/alsa_seq/midi_in.hpp>
#include <libremidi/backends/alsa_seq/observer.hpp>
#include <libremidi/backends/alsa_seq_ump/config.hpp>
#include <libremidi/backends/alsa_seq_ump/midi_out.hpp>

#include <unistd.h>

#include <string_view>

NAMESPACE_LIBREMIDI
{
template <>
inline std::unique_ptr<observer_api>
make<alsa_seq::observer_impl<alsa_seq_ump::observer_configuration>>(
    libremidi::observer_configuration&& conf,
    libremidi::alsa_seq_ump::observer_configuration&& api)
{
  if (api.manual_poll)
    return std::make_unique<alsa_seq::observer_manual<alsa_seq_ump::observer_configuration>>(
        std::move(conf), std::move(api));
  else
    return std::make_unique<alsa_seq::observer_threaded<alsa_seq_ump::observer_configuration>>(
        std::move(conf), std::move(api));
}

template <>
inline std::unique_ptr<midi_in_api> make<
    alsa_seq::midi_in_impl<libremidi::ump_input_configuration, alsa_seq_ump::input_configuration>>(
    libremidi::ump_input_configuration&& conf, libremidi::alsa_seq_ump::input_configuration&& api)
{
  if (api.manual_poll)
    return std::make_unique<alsa_seq::midi_in_alsa_manual<
        libremidi::ump_input_configuration, alsa_seq_ump::input_configuration>>(
        std::move(conf), std::move(api));
  else
    return std::make_unique<alsa_seq::midi_in_alsa_threaded<
        libremidi::ump_input_configuration, alsa_seq_ump::input_configuration>>(
        std::move(conf), std::move(api));
}

}

NAMESPACE_LIBREMIDI::alsa_seq_ump
{
struct backend
{
  using midi_in = alsa_seq::midi_in_impl<
      libremidi::ump_input_configuration, alsa_seq_ump::input_configuration>;
  using midi_out = alsa_seq_ump::midi_out_impl;
  using midi_observer = alsa_seq::observer_impl<alsa_seq_ump::observer_configuration>;
  using midi_in_configuration = alsa_seq_ump::input_configuration;
  using midi_out_configuration = alsa_seq_ump::output_configuration;
  using midi_observer_configuration = alsa_seq_ump::observer_configuration;
  static const constexpr auto API = libremidi::API::ALSA_SEQ_UMP;
  static const constexpr std::string_view name = "alsa_seq_ump";
  static const constexpr std::string_view display_name = "ALSA (sequencer, UMP)";

  static inline bool available() noexcept
  {
    static const libasound& snd = libasound::instance();
    if (!snd.available || !snd.seq.available || !snd.seq.ump.available || !snd.ump.available)
      return false;

    return ::access("/dev/snd/seq", F_OK | R_OK | W_OK) == 0;
  }
};
}

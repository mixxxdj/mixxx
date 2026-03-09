#pragma once
#include <libremidi/backends/jack_ump/midi_in.hpp>
#include <libremidi/backends/jack_ump/midi_out.hpp>
#include <libremidi/backends/jack_ump/observer.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI::jack_ump
{
struct backend
{
  using midi_in = midi_in_jack;
  using midi_out = midi_out_jack;
  using midi_observer = observer_jack;
  using midi_in_configuration = jack_ump::input_configuration;
  using midi_out_configuration = jack_ump::output_configuration;
  using midi_observer_configuration = jack_ump::observer_configuration;
  static const constexpr auto API = libremidi::API::JACK_UMP;
  static const constexpr std::string_view name = "jack_ump";
  static const constexpr std::string_view display_name = "JACK (UMP)";

  static inline bool available() noexcept
  {
#if LIBREMIDI_WEAKJACK
    if (WeakJack::instance().available() != 0)
      return false;
#endif

    int major, minor, micro, patch;
    jack_get_version(&major, &minor, &micro, &patch);
    return (major >= 3 && minor >= 1 && micro >= 4);
  }
};
}

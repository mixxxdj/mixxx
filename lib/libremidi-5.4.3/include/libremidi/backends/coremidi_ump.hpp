#pragma once
#include <libremidi/backends/coremidi_ump/midi_in.hpp>
#include <libremidi/backends/coremidi_ump/midi_out.hpp>
#include <libremidi/backends/coremidi_ump/observer.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI::coremidi_ump
{
struct backend
{
  using midi_in = midi_in_impl;
  using midi_out = midi_out_impl;
  using midi_observer = observer_impl;
  using midi_in_configuration = coremidi_ump::input_configuration;
  using midi_out_configuration = coremidi_ump::output_configuration;
  using midi_observer_configuration = coremidi_ump::observer_configuration;
  static const constexpr auto API = libremidi::API::COREMIDI_UMP;
  static const constexpr std::string_view name = "core_ump";
  static const constexpr std::string_view display_name = "CoreMIDI UMP";

  static constexpr inline bool available() noexcept { return true; /* todo? */ }
};
}

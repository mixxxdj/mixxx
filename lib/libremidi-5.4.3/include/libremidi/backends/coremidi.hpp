#pragma once
#include <libremidi/backends/coremidi/midi_in.hpp>
#include <libremidi/backends/coremidi/midi_out.hpp>
#include <libremidi/backends/coremidi/observer.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI
{
struct core_backend
{
  using midi_in = midi_in_core;
  using midi_out = midi_out_core;
  using midi_observer = observer_core;
  using midi_in_configuration = coremidi_input_configuration;
  using midi_out_configuration = coremidi_output_configuration;
  using midi_observer_configuration = coremidi_observer_configuration;
  static const constexpr auto API = libremidi::API::COREMIDI;
  static const constexpr std::string_view name = "core";
  static const constexpr std::string_view display_name = "CoreMIDI";

  static constexpr inline bool available() noexcept { return true; }
};
}

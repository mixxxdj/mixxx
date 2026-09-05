#pragma once
#include <libremidi/backends/kdmapi/midi_in.hpp>
#include <libremidi/backends/kdmapi/midi_out.hpp>
#include <libremidi/backends/kdmapi/observer.hpp>

#include <string_view>

//*********************************************************************//
//  API: OmniMIDI KDMAPI (Keppy's Direct MIDI API)
//*********************************************************************//

// OmniMIDI is a low-latency MIDI synthesizer driver for Windows.
// KDMAPI provides a direct interface to OmniMIDI, bypassing the
// Windows Multimedia API for lower latency.
//
// Note: KDMAPI is output-only. The midi_in class is a placeholder
// that always fails - use a different backend for MIDI input.
//
// https://github.com/KeppySoftware/OmniMIDI

namespace libremidi
{

struct kdmapi_backend
{
  using midi_in = kdmapi::midi_in;
  using midi_out = kdmapi::midi_out;
  using midi_observer = kdmapi::observer;
  using midi_in_configuration = kdmapi::input_configuration;
  using midi_out_configuration = kdmapi::output_configuration;
  using midi_observer_configuration = kdmapi::observer_configuration;
  static const constexpr auto API = libremidi::API::KDMAPI;
  static const constexpr std::string_view name = "kdmapi";
  static const constexpr std::string_view display_name = "OmniMIDI (KDMAPI)";

  static inline bool available() noexcept
  {
    return kdmapi::kdmapi_loader::instance().is_available();
  }
};

}


#pragma once
#include <libremidi/backends/winmm/midi_in.hpp>
#include <libremidi/backends/winmm/midi_out.hpp>
#include <libremidi/backends/winmm/observer.hpp>

#include <string_view>

// Default for Windows is to add an identifier to the port names; this
// flag can be defined (e.g. in your project file) to disable this behaviour.
// #define LIBREMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES

//*********************************************************************//
//  API: Windows Multimedia Library (MM)
//*********************************************************************//

// API information deciphered from:
//  -
//  http://msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_midi_reference.asp

// Thanks to Jean-Baptiste Berruchon for the sysex code.
NAMESPACE_LIBREMIDI
{

struct winmm_backend
{
  using midi_in = midi_in_winmm;
  using midi_out = midi_out_winmm;
  using midi_observer = observer_winmm;
  using midi_in_configuration = winmm_input_configuration;
  using midi_out_configuration = winmm_output_configuration;
  using midi_observer_configuration = winmm_observer_configuration;
  static const constexpr auto API = libremidi::API::WINDOWS_MM;
  static const constexpr std::string_view name = "winmm";
  static const constexpr std::string_view display_name = "Windows Multimedia";

  static constexpr inline bool available() noexcept { return true; }
};
}

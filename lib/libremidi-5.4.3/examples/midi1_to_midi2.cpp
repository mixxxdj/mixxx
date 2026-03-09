#include "utils.hpp"

#include <libremidi/backends.hpp>
#include <libremidi/libremidi.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <iostream>

/**
 * This example shows upscale of a MIDI 1 API into MIDI 2 messages
 */

int main()
try
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  // The observer object enumerates available inputs and outputs
  // jack is a MIDI 1 API (there's also jack_ump which leverages the newer
  // JACK / PipeWire MIDI 2 support).
  libremidi::observer obs{{}, libremidi::jack_observer_configuration{}};
  auto pi = obs.get_input_ports();
  auto po = obs.get_output_ports();
  if (pi.empty() || po.empty())
    throw std::runtime_error("No MIDI in / out pair available");

  // Create a midi in
  auto on_ump = [&](const libremidi::ump& message) { std::cerr << message << "\n"; };
  libremidi::midi_in midiin{{.on_message = on_ump}, libremidi::jack_input_configuration{}};

  if (auto err = midiin.open_port(pi[0]); err != stdx::error{})
    err.throw_exception();

  // Wait until we exit
  char input;
  std::cin.get(input);
}
catch (const std::exception& error)
{
  std::cerr << error.what() << std::endl;
  return EXIT_FAILURE;
}

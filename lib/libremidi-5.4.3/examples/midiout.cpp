//*****************************************//
//  midiout.cpp
//  by Gary Scavone, 2003-2004.
//
//  Simple program to test MIDI output.
//
//*****************************************//

#include "utils.hpp"

#include <libremidi/libremidi.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <array>
#include <chrono>
#include <thread>

int main(int argc, const char** argv)
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  using namespace std::literals;

  // Read command line arguments
  libremidi::examples::arguments args{argc, argv};

  libremidi::midi_out midiout{{}, libremidi::midi_out_configuration_for(args.api)};

  // Call function to select port.
  if (!args.open_port(midiout))
    return 1;

  // Send out a series of MIDI messages.

  // Program change: 192, 5
  midiout.send_message(192, 5);

  std::this_thread::sleep_for(500ms);

  midiout.send_message(0xF1, 60);

  // Control Change: 176, 7, 100 (volume)
  midiout.send_message(176, 7, 100);

  // Note On: 144, 64, 90
  midiout.send_message(144, 64, 90);

  std::this_thread::sleep_for(500ms);

  // Note Off: 128, 64, 40
  midiout.send_message(128, 64, 40);

  std::this_thread::sleep_for(500ms);

  // Control Change: 176, 7, 40
  midiout.send_message(176, 7, 40);

  std::this_thread::sleep_for(500ms);

  // Sysex: 240, 67, 4, 3, 2, 247
  midiout.send_message(std::to_array<unsigned char>({240, 67, 4, 3, 2, 247}));

  return 0;
}

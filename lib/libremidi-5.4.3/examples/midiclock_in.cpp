//*****************************************//
//  midiclock.cpp
//
//  Simple program to test MIDI clock sync.  Run midiclock_in in one
//  console and midiclock_out in the other, make sure to choose
//  options that connect the clocks between programs on your platform.
//
//  (C)2016 Refer to README.md in this archive for copyright.
//
//*****************************************//

#include "utils.hpp"

#include <libremidi/libremidi.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <cstdlib>
#include <iostream>

int main(int argc, const char** argv)
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  unsigned int clock_count = 0;
  auto midi_callback = [&](const libremidi::message& message) {
    // Ignore longer messages
    if (message.size() != 1)
      return;

    unsigned int msg = message[0];
    if (msg == 0xFA)
      std::cout << "START received" << std::endl;
    if (msg == 0xFB)
      std::cout << "CONTINUE received" << std::endl;
    if (msg == 0xFC)
      std::cout << "STOP received" << std::endl;
    if (msg == 0xF8)
    {
      if (++clock_count == 24)
      {
        double bpm = 60.0 / 24.0 / message.timestamp;
        std::cout << "One beat, estimated BPM = " << bpm << std::endl;
        clock_count = 0;
      }
    }
    else
      clock_count = 0;
  };

  // Read command line arguments
  libremidi::examples::arguments args{argc, argv};
  libremidi::midi_in midiin{
      {
          // Setup a callback
          .on_message = midi_callback,

          // Don't ignore sysex, timing, or active sensing messages.
          .ignore_sysex = false,
          .ignore_timing = false,
          .ignore_sensing = false,
      },
      libremidi::midi_in_configuration_for(args.api)};

  if (!args.open_port(midiin))
    return 1;

  std::cout << "\nReading MIDI input ... press <enter> to quit.\n";
  char input;
  std::cin.get(input);
  return 0;
}

//*****************************************//
//  cmidiin.cpp
//  by Gary Scavone, 2003-2004.
//
//  Simple program to test MIDI input and
//  use of a user callback function.
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

  // Read command line arguments
  libremidi::examples::arguments args{argc, argv};

  libremidi::midi_in midiin{
      {.on_message = {},
       .on_raw_data =
           [](std::span<const uint8_t> data, libremidi::timestamp t) {
    std::cout << std::dec << t << ": " << std::hex;
    for (auto byte : data)
      std::cout << int(byte) << " ";
    std::cout << std::endl;
  }},
      libremidi::midi_in_configuration_for(
          args.api)}; // Passing the second configuration argument can be skipped to use the default MIDI API

  if (!args.open_port(midiin))
    return 1;

  std::cout << "\nReading MIDI input ... press <enter> to quit.\n";
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

// midiprobe.cpp
//
// Simple program to check MIDI inputs and outputs.
//
// by Gary Scavone, 2003-2012.

#include "utils.hpp"

#include <libremidi/libremidi.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

void enumerate_api(libremidi::API api)
{
  std::string_view api_name = libremidi::get_api_display_name(api);
  std::cout << "Displaying ports for: " << api_name << std::endl;

  // On Windows 10, apparently the MIDI devices aren't exactly available as soon as the app open...
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  libremidi::observer midi{
      {.track_hardware = true, .track_virtual = true}, libremidi::observer_configuration_for(api)};

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    // Check inputs.
    auto ports = midi.get_input_ports();
    std::cout << ports.size() << " MIDI input sources:\n";
    int i = 0;
    for (auto& port : ports)
      std::cout << " - " << i++ << ": " << port << '\n';
  }

  {
    // Check outputs.
    auto ports = midi.get_output_ports();
    std::cout << ports.size() << " MIDI output sinks:\n";
    int i = 0;
    for (auto& port : ports)
      std::cout << " - " << i++ << ": " << port << '\n';
  }

  std::cout << "\n";
}

int main()
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif
  for (auto& api : libremidi::available_apis())
    enumerate_api(api);
  for (auto& api : libremidi::available_ump_apis())
    enumerate_api(api);
  return 0;
}

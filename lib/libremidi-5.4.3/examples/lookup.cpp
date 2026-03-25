// midiprobe.cpp
//
// Simple program to check MIDI inputs and outputs.
//
// by Gary Scavone, 2003-2012.

#include "utils.hpp"

#include <libremidi/libremidi.hpp>
#include <libremidi/port_comparison.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

void lookup_api(libremidi::API api, const libremidi::input_port& searched)
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
    auto res
        = libremidi::find_closest_port(searched, std::span<const libremidi::input_port>(ports));
    if (res.found) {
      std::cout << "Found: " << *res.port << "\n";
    }
  }
}

int main()
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  // This will find the port that is closest to this.
  // For instance, given a Launchpad and a Launchpad Mini,
  // the launchpad will be returned. Otherwise, the mini will be returned.
  libremidi::input_port searched{{.port_name = "launchpad"}};

  for (auto& api : libremidi::available_apis())
    lookup_api(api, searched);
  for (auto& api : libremidi::available_ump_apis())
    lookup_api(api, searched);
  return 0;
}

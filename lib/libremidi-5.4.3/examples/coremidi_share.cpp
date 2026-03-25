#include "utils.hpp"

#include <libremidi/configurations.hpp>
#include <libremidi/libremidi.hpp>

#include <CoreMIDI/CoreMIDI.h>

#include <chrono>
#include <iostream>
#include <thread>

struct my_app
{
  MIDIClientRef handle;

  std::vector<libremidi::midi_in> midiin;
  std::vector<libremidi::midi_out> midiout;

  my_app()
  {
    auto callback = [&](int port, const libremidi::message& msg) {
      std::cout << msg << std::endl;
      midiout[port].send_message(msg);
    };

    auto res = MIDIClientCreate(CFSTR("My App"), 0, 0, &handle);
    if (res != noErr)
      throw std::runtime_error("Could not start CoreMIDI");

    // Create our configuration
    auto api_input_config = libremidi::coremidi_input_configuration{.context = handle};
    auto api_output_config = libremidi::coremidi_output_configuration{.context = handle};

    // Create 16 inputs and 16 outputs
    for (int i = 0; i < 16; i++)
    {
      midiin.emplace_back(
          libremidi::input_configuration{
              .on_message = [=](const libremidi::message& msg) { callback(i, msg); }},
          api_input_config);
      midiin[i].open_virtual_port("Input: " + std::to_string(i));

      midiout.emplace_back(libremidi::output_configuration{}, api_output_config);
      midiout[i].open_virtual_port("Output: " + std::to_string(i));
    }
  }

  ~my_app() { MIDIClientDispose(handle); }
};

int main()
{
  my_app app{};

  for (;;)
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

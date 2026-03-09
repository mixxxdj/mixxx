#include "utils.hpp"

#include <libremidi/configurations.hpp>
#include <libremidi/libremidi.hpp>

#include <emscripten.h>

#include <array>
#include <iostream>
#include <memory>

/**
 * Note: due to Javascript being mostly async,
 * we need to use things in an async way here too.
 */
int main(int argc, char**)
{
  std::vector<libremidi::midi_in> inputs;
  std::vector<libremidi::midi_out> outputs;

  libremidi::observer_configuration callbacks{
      .input_added =
          [&](const libremidi::input_port& id) {
    std::cout << "MIDI Input connected: " << id.port_name << std::endl;

    auto conf = libremidi::input_configuration{
        .on_message = [](const libremidi::message& msg) { std::cout << msg << std::endl; }};
    auto& input = inputs.emplace_back(conf, libremidi::emscripten_input_configuration{});
    input.open_port(id);
  },

      .input_removed =
          [&](const libremidi::input_port& id) {
    std::cout << "MIDI Input removed: " << id.port_name << std::endl;
  },

      .output_added =
          [&](const libremidi::output_port& id) {
    std::cout << "MIDI Output connected: " << id.port_name << std::endl;

    libremidi::midi_out output{};
    output.open_port(id);
    output.send_message(0x90, 64, 100);
    output.send_message(0x80, 64, 100);
  },

      .output_removed = [&](const libremidi::output_port& id) {
    std::cout << "MIDI Output removed: " << id.port_name << std::endl;
  }};

  libremidi::observer obs{
      std::move(callbacks), libremidi::observer_configuration_for(libremidi::API::WEBMIDI)};

  emscripten_set_main_loop([] { }, 60, 1);
}

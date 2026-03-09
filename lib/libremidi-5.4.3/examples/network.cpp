#include "utils.hpp"

#include <libremidi/configurations.hpp>
#include <libremidi/libremidi.hpp>

#include <boost/asio.hpp>

int main(int argc, const char** argv)
{
  using namespace std::literals;
  libremidi::examples::arguments args{argc, argv};

  boost::asio::io_context ctx;

  // MIDI 1
  {
    libremidi::net::dgram_input_configuration in_apiconf{
        .client_name = "libremidi",
        .protocol = libremidi::net::protocol::OSC_MIDI,
        .accept = "0.0.0.0",
        .port = 5677,
        .io_context = &ctx};

    libremidi::midi_in midiin{
        {.on_message = [](const libremidi::message& m) { std::cerr << m << std::endl; }},
        in_apiconf};
    midiin.open_virtual_port("/midi");

    libremidi::net::dgram_output_configuration out_apiconf{
        .client_name = "libremidi",
        .protocol = libremidi::net::protocol::OSC_MIDI,
        .host = "127.0.0.1",
        .port = 5677,
        .broadcast = false,
        .io_context = &ctx};

    libremidi::midi_out midiout{{}, out_apiconf};

    if (auto err = midiout.open_virtual_port("/midi"); err != stdx::error{})
      err.throw_exception();

    if (auto err = midiout.send_message(144, 127, 64); err != stdx::error{})
      err.throw_exception();

    ctx.run_one();
  }

  ctx.restart();

  // MIDI 2
  {
    libremidi::net_ump::dgram_input_configuration in_apiconf{
        .client_name = "libremidi",
        .protocol = libremidi::net_ump::protocol::OSC_MIDI2,
        .accept = "0.0.0.0",
        .port = 5688,
        .io_context = &ctx};

    libremidi::midi_in midiin{
        {.on_message = [](const libremidi::ump& m) { std::cerr << m << std::endl; }}, in_apiconf};
    midiin.open_virtual_port("/ump");

    libremidi::net_ump::dgram_output_configuration out_apiconf{
        .client_name = "libremidi",
        .protocol = libremidi::net_ump::protocol::OSC_MIDI2,
        .host = "127.0.0.1",
        .port = 5688,
        .broadcast = false,
        .io_context = &ctx};

    libremidi::midi_out midiout{{}, out_apiconf};

    if (auto err = midiout.open_virtual_port("/ump"); err != stdx::error{})
      err.throw_exception();

    int64_t ump = cmidi2_ump_midi2_cc(1, 1, 34, 53404);
    if (auto err = midiout.send_ump(ump); err != stdx::error{})
      err.throw_exception();

    ctx.run_one();
    ctx.run_one();
  }

  // MIDI 1 no context
  {
    libremidi::net::dgram_input_configuration in_apiconf{
        .client_name = "libremidi",
        .protocol = libremidi::net::protocol::OSC_MIDI,
        .accept = "0.0.0.0",
        .port = 5677};

    libremidi::midi_in midiin{
        {.on_message = [](const libremidi::message& m) { std::cerr << m << std::endl; }},
        in_apiconf};
    midiin.open_virtual_port("/midi");

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    libremidi::net::dgram_output_configuration out_apiconf{
        .client_name = "libremidi",
        .protocol = libremidi::net::protocol::OSC_MIDI,
        .host = "127.0.0.1",
        .port = 5677,
        .broadcast = false,
        .io_context = &ctx};

    libremidi::midi_out midiout{{}, out_apiconf};

    if (auto err = midiout.open_virtual_port("/midi"); err != stdx::error{})
      err.throw_exception();

    if (auto err = midiout.send_message(144, 127, 64); err != stdx::error{})
      err.throw_exception();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

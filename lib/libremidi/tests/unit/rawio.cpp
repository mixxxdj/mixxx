#include "../include_catch.hpp"

#include <libremidi/configurations.hpp>
#include <libremidi/libremidi.hpp>

TEST_CASE("rawio midi1 roundtrip", "[rawio]")
{
  // The callback that the library will give us to feed bytes into
  libremidi::rawio_input_configuration::receive_callback on_receive;

  // Received messages
  std::vector<libremidi::message> received;

  libremidi::midi_in midiin{
      libremidi::input_configuration{
          .on_message = [&](const libremidi::message& m) { received.push_back(m); }},
      libremidi::rawio_input_configuration{
          .set_receive_callback = [&](auto cb) { on_receive = std::move(cb); },
          .stop_receive = [&] { on_receive = nullptr; }}};

  REQUIRE(midiin.get_current_api() == libremidi::API::RAW_IO);

  // Bytes written by midi_out
  std::vector<uint8_t> written;

  libremidi::midi_out midiout{
      libremidi::output_configuration{},
      libremidi::rawio_output_configuration{
          .write_bytes = [&](std::span<const uint8_t> bytes) -> stdx::error {
    written.assign(bytes.begin(), bytes.end());
    return {};
  }}};

  REQUIRE(midiout.get_current_api() == libremidi::API::RAW_IO);

  midiin.open_virtual_port("test");
  midiout.open_virtual_port("test");

  SECTION("note on roundtrip")
  {
    // Send a note-on through midi_out
    midiout.send_message(0x90, 60, 100);

    // Verify the output wrote correct bytes
    REQUIRE(written.size() == 3);
    REQUIRE(written[0] == 0x90);
    REQUIRE(written[1] == 60);
    REQUIRE(written[2] == 100);

    // Feed those bytes back into midi_in (simulating a loopback transport)
    REQUIRE(on_receive);
    on_receive(written, 0);

    // Verify the message was received and parsed
    REQUIRE(received.size() == 1);
    REQUIRE(received[0].bytes.size() == 3);
    REQUIRE(received[0].bytes[0] == 0x90);
    REQUIRE(received[0].bytes[1] == 60);
    REQUIRE(received[0].bytes[2] == 100);
  }

  SECTION("multiple messages")
  {
    // Note on
    midiout.send_message(0x90, 60, 100);
    REQUIRE(on_receive);
    on_receive(written, 0);

    // Note off
    midiout.send_message(0x80, 60, 0);
    on_receive(written, 0);

    // CC
    midiout.send_message(0xB0, 7, 100);
    on_receive(written, 0);

    REQUIRE(received.size() == 3);
    REQUIRE(received[0].bytes[0] == 0x90);
    REQUIRE(received[1].bytes[0] == 0x80);
    REQUIRE(received[2].bytes[0] == 0xB0);
    REQUIRE(received[2].bytes[1] == 7);
    REQUIRE(received[2].bytes[2] == 100);
  }

  SECTION("close port calls stop_receive")
  {
    REQUIRE(on_receive);
    midiin.close_port();
    REQUIRE_FALSE(on_receive);
  }
}

TEST_CASE("rawio midi2 ump roundtrip", "[rawio]")
{
  // The callback that the library will give us to feed UMP words into
  libremidi::rawio_ump_input_configuration::receive_callback on_receive;

  // Received UMP messages
  std::vector<libremidi::ump> received;

  libremidi::midi_in midiin{
      libremidi::ump_input_configuration{
          .on_message = [&](const libremidi::ump& m) { received.push_back(m); }},
      libremidi::rawio_ump_input_configuration{
          .set_receive_callback = [&](auto cb) { on_receive = std::move(cb); },
          .stop_receive = [&] { on_receive = nullptr; }}};

  REQUIRE(midiin.get_current_api() == libremidi::API::RAW_IO_UMP);

  // Words written by midi_out
  std::vector<uint32_t> written;

  libremidi::midi_out midiout{
      libremidi::output_configuration{},
      libremidi::rawio_ump_output_configuration{
          .write_ump = [&](std::span<const uint32_t> words) -> stdx::error {
    written.assign(words.begin(), words.end());
    return {};
  }}};

  REQUIRE(midiout.get_current_api() == libremidi::API::RAW_IO_UMP);

  midiin.open_virtual_port("test");
  midiout.open_virtual_port("test");

  SECTION("ump roundtrip")
  {
    // Send a MIDI 2.0 note-on UMP (type 4, group 0, channel 0, note 60, velocity 0xC000)
    uint32_t ump[2] = {0x40900000 | 60, 0xC0000000};
    midiout.send_ump(ump, 2);

    // Verify output
    REQUIRE(written.size() == 2);

    // Feed back into input
    REQUIRE(on_receive);
    on_receive(written, 0);

    // Verify reception
    REQUIRE(received.size() == 1);
    REQUIRE(received[0].data[0] == ump[0]);
    REQUIRE(received[0].data[1] == ump[1]);
  }
}

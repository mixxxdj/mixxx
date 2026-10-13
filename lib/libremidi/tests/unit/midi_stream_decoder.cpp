#include "../include_catch.hpp"

#include <libremidi/detail/midi_stream_decoder.hpp>

#include <vector>

// Helper to collect decoded MIDI 1 messages
struct midi1_collector
{
  libremidi::input_configuration configuration;
  std::vector<libremidi::message> messages;

  midi1_collector()
  {
    configuration.on_message = [this](libremidi::message&& msg) {
      messages.push_back(std::move(msg));
    };
    configuration.timestamps = libremidi::timestamp_mode::NoTimestamp;
    configuration.ignore_sysex = false;
    configuration.ignore_timing = false;
    configuration.ignore_sensing = false;
  }

  auto make_state_machine() { return libremidi::midi1::input_state_machine{configuration}; }
};

// Helper to collect decoded MIDI 2 UMP messages
struct midi2_collector
{
  libremidi::ump_input_configuration configuration;
  std::vector<libremidi::ump> messages;

  midi2_collector()
  {
    configuration.on_message = [this](libremidi::ump&& msg) {
      messages.push_back(std::move(msg));
    };
    configuration.timestamps = libremidi::timestamp_mode::NoTimestamp;
    configuration.ignore_sysex = false;
    configuration.ignore_timing = false;
    configuration.ignore_sensing = false;
    configuration.midi1_channel_events_to_midi2 = false;
  }

  auto make_state_machine() { return libremidi::midi2::input_state_machine{configuration}; }
};

// ============================================================================
// MIDI 1 input_state_machine
// ============================================================================

TEST_CASE("midi1: single channel messages via on_bytes", "[midi1][state_machine]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  SECTION("Note On")
  {
    const uint8_t bytes[] = {0x90, 0x3C, 0x7F}; // ch1, note 60, vel 127
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 3);
    REQUIRE(c.messages[0][0] == 0x90);
    REQUIRE(c.messages[0][1] == 0x3C);
    REQUIRE(c.messages[0][2] == 0x7F);
  }

  SECTION("Note Off")
  {
    const uint8_t bytes[] = {0x80, 0x3C, 0x40};
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::NOTE_OFF);
  }

  SECTION("Control Change")
  {
    const uint8_t bytes[] = {0xB0, 0x07, 0x64}; // ch1, CC 7, value 100
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::CONTROL_CHANGE);
    REQUIRE(c.messages[0].get_channel() == 1);
  }

  SECTION("Program Change (2 bytes)")
  {
    const uint8_t bytes[] = {0xC0, 0x05};
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 2);
    REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::PROGRAM_CHANGE);
  }

  SECTION("Pitch Bend")
  {
    const uint8_t bytes[] = {0xE0, 0x00, 0x40}; // center value
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::PITCH_BEND);
  }

  SECTION("Aftertouch (2 bytes)")
  {
    const uint8_t bytes[] = {0xD3, 0x50}; // ch4
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 2);
    REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::AFTERTOUCH);
    REQUIRE(c.messages[0].get_channel() == 4);
  }
}

TEST_CASE("midi1: multiple messages in one buffer via on_bytes_multi", "[midi1][state_machine]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  SECTION("two Note On messages back to back")
  {
    const uint8_t bytes[] = {
        0x90, 0x3C, 0x7F, // Note On ch1 C4
        0x90, 0x40, 0x60, // Note On ch1 E4
    };
    sm.on_bytes_multi(bytes, 0);

    REQUIRE(c.messages.size() == 2);
    REQUIRE(c.messages[0][1] == 0x3C);
    REQUIRE(c.messages[1][1] == 0x40);
  }

  SECTION("mixed message types")
  {
    const uint8_t bytes[] = {
        0x90, 0x3C, 0x7F, // Note On (3 bytes)
        0xC0, 0x05,       // Program Change (2 bytes)
        0xB0, 0x07, 0x64, // CC (3 bytes)
    };
    sm.on_bytes_multi(bytes, 0);

    REQUIRE(c.messages.size() == 3);
    REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::NOTE_ON);
    REQUIRE(c.messages[1].get_message_type() == libremidi::message_type::PROGRAM_CHANGE);
    REQUIRE(c.messages[2].get_message_type() == libremidi::message_type::CONTROL_CHANGE);
  }

  SECTION("single-byte real-time messages interleaved")
  {
    const uint8_t bytes[] = {
        0xFA,             // Start (1 byte)
        0x90, 0x3C, 0x7F, // Note On (3 bytes)
        0xFC,             // Stop (1 byte)
    };
    sm.on_bytes_multi(bytes, 0);

    REQUIRE(c.messages.size() == 3);
    REQUIRE(c.messages[0][0] == 0xFA);
    REQUIRE(c.messages[1].get_message_type() == libremidi::message_type::NOTE_ON);
    REQUIRE(c.messages[2][0] == 0xFC);
  }
}

TEST_CASE("midi1: SysEx handling", "[midi1][state_machine]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  SECTION("complete SysEx in one buffer via on_bytes")
  {
    const uint8_t bytes[] = {0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7};
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 6);
    REQUIRE(c.messages[0].front() == 0xF0);
    REQUIRE(c.messages[0].back() == 0xF7);
  }

  SECTION("complete SysEx in one buffer via on_bytes_multi")
  {
    const uint8_t bytes[] = {0xF0, 0x00, 0x01, 0x02, 0xF7};
    sm.on_bytes_multi(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 5);
  }

  SECTION("SysEx split across two buffers via on_bytes")
  {
    const uint8_t part1[] = {0xF0, 0x7E, 0x7F};
    const uint8_t part2[] = {0x09, 0x01, 0xF7};

    sm.on_bytes(part1, 0);
    REQUIRE(c.messages.empty()); // not complete yet

    sm.on_bytes(part2, 0);
    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 6);
    REQUIRE(c.messages[0][0] == 0xF0);
    REQUIRE(c.messages[0][5] == 0xF7);
  }

  SECTION("SysEx split across three buffers")
  {
    sm.on_bytes({std::vector<uint8_t>{0xF0, 0x01}}, 0);
    REQUIRE(c.messages.empty());

    sm.on_bytes({std::vector<uint8_t>{0x02, 0x03}}, 0);
    REQUIRE(c.messages.empty());

    sm.on_bytes({std::vector<uint8_t>{0x04, 0xF7}}, 0);
    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 6);
  }
}

TEST_CASE("midi1: SysEx filtering", "[midi1][state_machine]")
{
  midi1_collector c;
  c.configuration.ignore_sysex = true;
  auto sm = c.make_state_machine();

  SECTION("SysEx ignored when filtered")
  {
    const uint8_t bytes[] = {0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7};
    sm.on_bytes(bytes, 0);
    REQUIRE(c.messages.empty());
  }

  SECTION("split SysEx ignored when filtered")
  {
    sm.on_bytes({std::vector<uint8_t>{0xF0, 0x01}}, 0);
    sm.on_bytes({std::vector<uint8_t>{0x02, 0xF7}}, 0);
    REQUIRE(c.messages.empty());
  }
}

TEST_CASE("midi1: timing and sensing filtering", "[midi1][state_machine]")
{
  SECTION("timing messages ignored by default")
  {
    midi1_collector c;
    c.configuration.ignore_timing = true;
    auto sm = c.make_state_machine();

    sm.on_bytes({std::vector<uint8_t>{0xF8}}, 0);          // timing clock
    sm.on_bytes({std::vector<uint8_t>{0xF1, 0x00}}, 0);    // MTC quarter frame
    REQUIRE(c.messages.empty());
  }

  SECTION("timing messages passed when not filtered")
  {
    midi1_collector c;
    c.configuration.ignore_timing = false;
    auto sm = c.make_state_machine();

    sm.on_bytes({std::vector<uint8_t>{0xF8}}, 0);
    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0][0] == 0xF8);
  }

  SECTION("active sensing ignored by default")
  {
    midi1_collector c;
    c.configuration.ignore_sensing = true;
    auto sm = c.make_state_machine();

    sm.on_bytes({std::vector<uint8_t>{0xFE}}, 0);
    REQUIRE(c.messages.empty());
  }

  SECTION("active sensing passed when not filtered")
  {
    midi1_collector c;
    c.configuration.ignore_sensing = false;
    auto sm = c.make_state_machine();

    sm.on_bytes({std::vector<uint8_t>{0xFE}}, 0);
    REQUIRE(c.messages.size() == 1);
  }
}

TEST_CASE("midi1: system common messages", "[midi1][state_machine]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  SECTION("Song Position Pointer (3 bytes)")
  {
    const uint8_t bytes[] = {0xF2, 0x00, 0x08};
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 3);
    REQUIRE(c.messages[0][0] == 0xF2);
  }

  SECTION("Song Select (2 bytes)")
  {
    const uint8_t bytes[] = {0xF3, 0x03};
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 2);
  }

  SECTION("Tune Request (1 byte)")
  {
    const uint8_t bytes[] = {0xF6};
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 1);
  }
}

TEST_CASE("midi1: timestamps", "[midi1][state_machine]")
{
  SECTION("timestamp is forwarded as-is from on_bytes")
  {
    midi1_collector c;
    auto sm = c.make_state_machine();

    sm.on_bytes({std::vector<uint8_t>{0x90, 0x3C, 0x7F}}, 42);
    REQUIRE(c.messages[0].timestamp == 42);
  }

  SECTION("timestamp<> template computes NoTimestamp as zero")
  {
    midi1_collector c;
    c.configuration.timestamps = libremidi::timestamp_mode::NoTimestamp;
    auto sm = c.make_state_machine();

    // Backends call timestamp<>() to compute the value before passing to on_bytes.
    // NoTimestamp always returns 0 regardless of the getter.
    constexpr libremidi::timestamp_backend_info info{};
    auto ts = sm.timestamp<info>([] { return int64_t{999}; }, 0);
    REQUIRE(ts == 0);
  }

  SECTION("timestamp<> template computes Absolute from getter")
  {
    midi1_collector c;
    c.configuration.timestamps = libremidi::timestamp_mode::Absolute;
    auto sm = c.make_state_machine();

    constexpr libremidi::timestamp_backend_info info{.has_absolute_timestamps = true};
    auto ts = sm.timestamp<info>([] { return int64_t{12345}; }, 0);
    REQUIRE(ts == 12345);
  }
}

TEST_CASE("midi1: reset clears state", "[midi1][state_machine]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  // Start a SysEx that doesn't finish
  sm.on_bytes({std::vector<uint8_t>{0xF0, 0x01, 0x02}}, 0);
  REQUIRE(c.messages.empty());

  // Reset discards the partial SysEx
  sm.reset();

  // Now a normal message should work
  sm.on_bytes({std::vector<uint8_t>{0x90, 0x3C, 0x7F}}, 0);
  REQUIRE(c.messages.size() == 1);
  REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::NOTE_ON);
}

TEST_CASE("midi1: raw_data callback", "[midi1][state_machine]")
{
  std::vector<std::vector<uint8_t>> raw_received;

  midi1_collector c;
  c.configuration.on_raw_data = [&](std::span<const uint8_t> data, int64_t) {
    raw_received.emplace_back(data.begin(), data.end());
  };
  auto sm = c.make_state_machine();

  const uint8_t bytes[] = {0x90, 0x3C, 0x7F, 0x80, 0x3C, 0x40};
  sm.on_bytes_multi(bytes, 0);

  // on_message splits into individual messages
  REQUIRE(c.messages.size() == 2);
  // on_raw_data gets the entire buffer as-is
  REQUIRE(raw_received.size() == 1);
  REQUIRE(raw_received[0].size() == 6);
}

// ============================================================================
// MIDI 2 input_state_machine
// ============================================================================

// Helper: build a MIDI 2 UMP Note On word
// UMP type 4 (MIDI 2 channel voice), group 0, status 0x90 (note on), channel 0
static uint32_t make_ump_midi2_note_on(uint8_t note, uint16_t velocity)
{
  // Word 0: type=4, group=0, status=0x9, channel=0, note, attribute_type=0
  uint32_t w0 = (0x40 << 24) | (0x90 << 16) | (note << 8);
  return w0;
}

// Helper: build a MIDI 1 channel voice UMP (type 2)
static uint32_t make_ump_midi1_note_on(uint8_t channel, uint8_t note, uint8_t velocity)
{
  // Word 0: type=2, group=0, status byte, data
  uint32_t w0 = (0x20 << 24) | ((0x90 | (channel & 0xF)) << 16) | (note << 8) | velocity;
  return w0;
}

// Helper: build a System message UMP (type 1)
static uint32_t make_ump_system(uint8_t status)
{
  uint32_t w0 = (0x10 << 24) | (status << 16);
  return w0;
}

TEST_CASE("midi2: single UMP message via on_bytes", "[midi2][state_machine]")
{
  midi2_collector c;
  auto sm = c.make_state_machine();

  SECTION("MIDI 2 channel voice (2 words)")
  {
    uint32_t words[2];
    words[0] = make_ump_midi2_note_on(60, 0xFFFF);
    words[1] = 0xFFFF0000; // velocity + attribute

    sm.on_bytes(words, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].get_type() == libremidi::midi2::message_type::MIDI_2_CHANNEL);
    REQUIRE(c.messages[0].data[0] == words[0]);
    REQUIRE(c.messages[0].data[1] == words[1]);
  }

  SECTION("System message (1 word)")
  {
    uint32_t words[1] = {make_ump_system(0xFA)}; // Start
    sm.on_bytes(words, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].get_type() == libremidi::midi2::message_type::SYSTEM);
  }
}

TEST_CASE("midi2: multiple UMP messages via on_bytes_multi", "[midi2][state_machine]")
{
  midi2_collector c;
  auto sm = c.make_state_machine();

  SECTION("two system messages")
  {
    uint32_t words[] = {
        make_ump_system(0xFA), // Start
        make_ump_system(0xFC), // Stop
    };
    sm.on_bytes_multi(std::span<const uint32_t>(words), 0);

    REQUIRE(c.messages.size() == 2);
  }

  SECTION("NOOP padding is skipped")
  {
    uint32_t words[] = {
        make_ump_system(0xFA), // Start
        0x00000000,            // NOOP/padding
        0x00000000,            // NOOP/padding
        make_ump_system(0xFC), // Stop
    };
    sm.on_bytes_multi(std::span<const uint32_t>(words), 0);

    REQUIRE(c.messages.size() == 2);
    REQUIRE(c.messages[0].data[0] == make_ump_system(0xFA));
    REQUIRE(c.messages[1].data[0] == make_ump_system(0xFC));
  }
}

TEST_CASE("midi2: MIDI 1 channel voice upscaling", "[midi2][state_machine]")
{
  SECTION("upscale enabled: MIDI 1 channel voice becomes MIDI 2")
  {
    midi2_collector c;
    c.configuration.midi1_channel_events_to_midi2 = true;
    auto sm = c.make_state_machine();

    uint32_t words[] = {make_ump_midi1_note_on(0, 60, 127)};
    sm.on_bytes(words, 0);

    REQUIRE(c.messages.size() == 1);
    // After upscaling, message type should be MIDI 2 channel voice
    REQUIRE(c.messages[0].get_type() == libremidi::midi2::message_type::MIDI_2_CHANNEL);
  }

  SECTION("upscale disabled: MIDI 1 channel voice stays as-is")
  {
    midi2_collector c;
    c.configuration.midi1_channel_events_to_midi2 = false;
    auto sm = c.make_state_machine();

    uint32_t words[] = {make_ump_midi1_note_on(0, 60, 127)};
    sm.on_bytes(words, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].get_type() == libremidi::midi2::message_type::MIDI_1_CHANNEL);
  }
}

TEST_CASE("midi2: filtering", "[midi2][state_machine]")
{
  SECTION("timing filtered")
  {
    midi2_collector c;
    c.configuration.ignore_timing = true;
    auto sm = c.make_state_machine();

    // Utility message (type 0) = timing
    uint32_t words[] = {0x00200000}; // type=0 utility, JR timestamp
    sm.on_bytes(words, 0);
    REQUIRE(c.messages.empty());

    // System timing clock
    uint32_t clock[] = {make_ump_system(0xF8)};
    sm.on_bytes(clock, 0);
    REQUIRE(c.messages.empty());

    // System MTC
    uint32_t mtc[] = {make_ump_system(0xF1)};
    sm.on_bytes(mtc, 0);
    REQUIRE(c.messages.empty());
  }

  SECTION("timing not filtered")
  {
    midi2_collector c;
    c.configuration.ignore_timing = false;
    auto sm = c.make_state_machine();

    uint32_t words[] = {0x00200000};
    sm.on_bytes(words, 0);
    REQUIRE(c.messages.size() == 1);
  }

  SECTION("active sensing filtered")
  {
    midi2_collector c;
    c.configuration.ignore_sensing = true;
    auto sm = c.make_state_machine();

    uint32_t words[] = {make_ump_system(0xFE)};
    sm.on_bytes(words, 0);
    REQUIRE(c.messages.empty());
  }

  SECTION("active sensing not filtered")
  {
    midi2_collector c;
    c.configuration.ignore_sensing = false;
    auto sm = c.make_state_machine();

    uint32_t words[] = {make_ump_system(0xFE)};
    sm.on_bytes(words, 0);
    REQUIRE(c.messages.size() == 1);
  }

  SECTION("SysEx 7 filtered")
  {
    midi2_collector c;
    c.configuration.ignore_sysex = true;
    auto sm = c.make_state_machine();

    // Type 3 = SysEx7 (2 words)
    uint32_t words[] = {0x30000000, 0x00000000};
    sm.on_bytes(words, 0);
    REQUIRE(c.messages.empty());
  }

  SECTION("SysEx 7 not filtered")
  {
    midi2_collector c;
    c.configuration.ignore_sysex = false;
    auto sm = c.make_state_machine();

    uint32_t words[] = {0x30000000, 0x00000000};
    sm.on_bytes(words, 0);
    REQUIRE(c.messages.size() == 1);
  }
}

TEST_CASE("midi2: byte-level input via on_bytes_multi(unsigned char)", "[midi2][state_machine]")
{
  midi2_collector c;
  auto sm = c.make_state_machine();

  // Build a system message as raw bytes (little-endian platform assumption
  // doesn't apply: UMP words are passed as uint32_t, reinterpreted from bytes)
  uint32_t word = make_ump_system(0xFA);
  auto* byte_ptr = reinterpret_cast<const unsigned char*>(&word);

  sm.on_bytes_multi({byte_ptr, 4}, 0);
  REQUIRE(c.messages.size() == 1);
}

TEST_CASE("midi2: raw_data callback", "[midi2][state_machine]")
{
  std::vector<std::vector<uint32_t>> raw_received;

  midi2_collector c;
  c.configuration.on_raw_data = [&](std::span<const uint32_t> data, int64_t) {
    raw_received.emplace_back(data.begin(), data.end());
  };
  auto sm = c.make_state_machine();

  uint32_t words[] = {make_ump_system(0xFA), make_ump_system(0xFC)};
  sm.on_bytes_multi(std::span<const uint32_t>(words), 0);

  // on_message splits into individual UMP messages
  REQUIRE(c.messages.size() == 2);
  // on_raw_data gets the entire buffer as-is
  REQUIRE(raw_received.size() == 1);
  REQUIRE(raw_received[0].size() == 2);
}

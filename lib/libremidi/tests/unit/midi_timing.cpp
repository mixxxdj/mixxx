#include "../include_catch.hpp"

#include <libremidi/detail/conversion.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

#include <vector>

// Round-trip test: build a MIDI timing message, feed it through the
// input_state_machine, and verify the decoded output matches.

namespace
{
struct midi1_collector
{
  libremidi::input_configuration configuration;
  std::vector<libremidi::message> messages;

  midi1_collector()
  {
    configuration.on_message
        = [this](libremidi::message&& msg) { messages.push_back(std::move(msg)); };
    configuration.timestamps = libremidi::timestamp_mode::NoTimestamp;
    configuration.ignore_sysex = false;
    configuration.ignore_timing = false;
    configuration.ignore_sensing = false;
  }

  auto make_state_machine() { return libremidi::midi1::input_state_machine{configuration}; }
};
}

// -- MTC Quarter Frame (0xF1) ------------------------------------------------
// Format: 0xF1 0nnndddd
//   nnn  = message type (0-7), selects which timecode component
//   dddd = 4-bit value
// A full SMPTE timecode is transmitted over 8 sequential quarter-frame messages
// (two per frame), cycling through:
//   0: frame count low nibble
//   1: frame count high nibble
//   2: seconds low nibble
//   3: seconds high nibble
//   4: minutes low nibble
//   5: minutes high nibble
//   6: hours low nibble
//   7: hours high nibble + SMPTE type (bits 1-0 of high nibble)

TEST_CASE("MTC Quarter Frame round-trip", "[timing][mtc]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  // Encode a full SMPTE timecode 01:23:45:12 at 25fps via 8 quarter-frame messages
  // 25fps = SMPTE type 1 (bits 5-4 of hours high nibble = 01)
  const uint8_t hours = 1, minutes = 23, seconds = 45, frames = 12;
  const uint8_t smpte_type = 1; // 25fps

  const uint8_t qf_messages[8][2] = {
      {0xF1, uint8_t(0x00 | (frames & 0x0F))},         // type 0: frame low
      {0xF1, uint8_t(0x10 | (frames >> 4))},            // type 1: frame high
      {0xF1, uint8_t(0x20 | (seconds & 0x0F))},         // type 2: seconds low
      {0xF1, uint8_t(0x30 | (seconds >> 4))},           // type 3: seconds high
      {0xF1, uint8_t(0x40 | (minutes & 0x0F))},         // type 4: minutes low
      {0xF1, uint8_t(0x50 | (minutes >> 4))},           // type 5: minutes high
      {0xF1, uint8_t(0x60 | (hours & 0x0F))},           // type 6: hours low
      {0xF1, uint8_t(0x70 | (smpte_type << 1) | (hours >> 4))}, // type 7: hours high + type
  };

  for (auto& qf : qf_messages)
    sm.on_bytes(qf, 0);

  REQUIRE(c.messages.size() == 8);

  // Decode back: reconstruct the timecode from the 8 messages
  uint8_t decoded_frames = 0, decoded_seconds = 0, decoded_minutes = 0, decoded_hours = 0;
  uint8_t decoded_type = 0;

  for (auto& msg : c.messages)
  {
    REQUIRE(msg.size() == 2);
    REQUIRE(msg[0] == 0xF1);

    uint8_t nibble_type = (msg[1] >> 4) & 0x07;
    uint8_t nibble_data = msg[1] & 0x0F;

    switch (nibble_type)
    {
      case 0:
        decoded_frames = (decoded_frames & 0xF0) | nibble_data;
        break;
      case 1:
        decoded_frames = (decoded_frames & 0x0F) | (nibble_data << 4);
        break;
      case 2:
        decoded_seconds = (decoded_seconds & 0xF0) | nibble_data;
        break;
      case 3:
        decoded_seconds = (decoded_seconds & 0x0F) | (nibble_data << 4);
        break;
      case 4:
        decoded_minutes = (decoded_minutes & 0xF0) | nibble_data;
        break;
      case 5:
        decoded_minutes = (decoded_minutes & 0x0F) | (nibble_data << 4);
        break;
      case 6:
        decoded_hours = (decoded_hours & 0xF0) | nibble_data;
        break;
      case 7:
        decoded_hours = (decoded_hours & 0x0F) | ((nibble_data & 0x01) << 4);
        decoded_type = (nibble_data >> 1) & 0x03;
        break;
    }
  }

  REQUIRE(decoded_frames == frames);
  REQUIRE(decoded_seconds == seconds);
  REQUIRE(decoded_minutes == minutes);
  REQUIRE(decoded_hours == hours);
  REQUIRE(decoded_type == smpte_type);
}

// -- Song Position Pointer (0xF2) --------------------------------------------
// Format: 0xF2 LSB MSB (14-bit value in MIDI beats = 1/16th notes)

TEST_CASE("Song Position Pointer round-trip", "[timing][spp]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  SECTION("position zero")
  {
    auto msg = libremidi::meta_events::song_position(0);
    sm.on_bytes({msg.bytes.data(), msg.bytes.size()}, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0][0] == 0xF2);
    uint16_t decoded = c.messages[0][1] | (c.messages[0][2] << 7);
    REQUIRE(decoded == 0);
  }

  SECTION("small value")
  {
    int position = 96; // beat 6 in 4/4 time (16 sixteenths per bar)
    auto msg = libremidi::meta_events::song_position(position);
    sm.on_bytes({msg.bytes.data(), msg.bytes.size()}, 0);

    REQUIRE(c.messages.size() == 1);
    uint16_t decoded = c.messages[0][1] | (c.messages[0][2] << 7);
    REQUIRE(decoded == position);
  }

  SECTION("maximum 14-bit value")
  {
    int position = 16383; // 0x3FFF
    auto msg = libremidi::meta_events::song_position(position);
    sm.on_bytes({msg.bytes.data(), msg.bytes.size()}, 0);

    REQUIRE(c.messages.size() == 1);
    uint16_t decoded = c.messages[0][1] | (c.messages[0][2] << 7);
    REQUIRE(decoded == 16383);
  }

  SECTION("round-trip via on_bytes_multi with surrounding messages")
  {
    int position = 1000;
    uint8_t bytes[] = {
        0xFA,                                                       // Start
        0xF2, uint8_t(position & 0x7F), uint8_t((position >> 7) & 0x7F), // SPP
        0xFB,                                                       // Continue
    };
    sm.on_bytes_multi(bytes, 0);

    REQUIRE(c.messages.size() == 3);
    REQUIRE(c.messages[0][0] == 0xFA);
    REQUIRE(c.messages[1][0] == 0xF2);
    REQUIRE(c.messages[2][0] == 0xFB);

    uint16_t decoded = c.messages[1][1] | (c.messages[1][2] << 7);
    REQUIRE(decoded == 1000);
  }
}

// -- Song Select (0xF3) ------------------------------------------------------
// Format: 0xF3 <song number 0-127>

TEST_CASE("Song Select round-trip", "[timing]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  for (uint8_t song : {0, 1, 42, 127})
  {
    c.messages.clear();
    const uint8_t bytes[] = {0xF3, song};
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 2);
    REQUIRE(c.messages[0][0] == 0xF3);
    REQUIRE(c.messages[0][1] == song);
  }
}

// -- Timing Clock (0xF8) -----------------------------------------------------
// Format: single byte 0xF8, sent 24 times per quarter note

TEST_CASE("Timing Clock round-trip", "[timing][clock]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  SECTION("single clock tick")
  {
    const uint8_t bytes[] = {0xF8};
    sm.on_bytes(bytes, 0);

    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0].size() == 1);
    REQUIRE(c.messages[0][0] == 0xF8);
  }

  SECTION("24 clock ticks per quarter note at 120 BPM")
  {
    // At 120 BPM, quarter note = 500ms, so clock interval = 500/24 ~ 20.83ms
    for (int i = 0; i < 24; ++i)
      sm.on_bytes({std::vector<uint8_t>{0xF8}}, 0);

    REQUIRE(c.messages.size() == 24);
    for (auto& msg : c.messages)
    {
      REQUIRE(msg.size() == 1);
      REQUIRE(msg[0] == 0xF8);
    }
  }
}

// -- Transport: Start (0xFA), Continue (0xFB), Stop (0xFC) -------------------

TEST_CASE("Transport messages round-trip", "[timing][transport]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  SECTION("Start")
  {
    sm.on_bytes({std::vector<uint8_t>{0xFA}}, 0);
    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0][0] == 0xFA);
    REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::START);
  }

  SECTION("Continue")
  {
    sm.on_bytes({std::vector<uint8_t>{0xFB}}, 0);
    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0][0] == 0xFB);
    REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::CONTINUE);
  }

  SECTION("Stop")
  {
    sm.on_bytes({std::vector<uint8_t>{0xFC}}, 0);
    REQUIRE(c.messages.size() == 1);
    REQUIRE(c.messages[0][0] == 0xFC);
    REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::STOP);
  }

  SECTION("typical transport sequence: Start, clocks, Stop")
  {
    const uint8_t sequence[] = {
        0xFA, // Start
        0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, // 6 clocks
        0xFC, // Stop
    };
    sm.on_bytes_multi(sequence, 0);

    REQUIRE(c.messages.size() == 8);
    REQUIRE(c.messages[0][0] == 0xFA);
    for (int i = 1; i <= 6; ++i)
      REQUIRE(c.messages[i][0] == 0xF8);
    REQUIRE(c.messages[7][0] == 0xFC);
  }
}

// -- Active Sensing (0xFE) ---------------------------------------------------

TEST_CASE("Active Sensing round-trip", "[timing]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  sm.on_bytes({std::vector<uint8_t>{0xFE}}, 0);
  REQUIRE(c.messages.size() == 1);
  REQUIRE(c.messages[0][0] == 0xFE);
  REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::ACTIVE_SENSING);
}

// -- System Reset (0xFF) -----------------------------------------------------

TEST_CASE("System Reset round-trip", "[timing]")
{
  midi1_collector c;
  auto sm = c.make_state_machine();

  sm.on_bytes({std::vector<uint8_t>{0xFF}}, 0);
  REQUIRE(c.messages.size() == 1);
  REQUIRE(c.messages[0][0] == 0xFF);
  REQUIRE(c.messages[0].get_message_type() == libremidi::message_type::SYSTEM_RESET);
}

// -- Filtering: timing messages are ignored by default -----------------------

TEST_CASE("Timing messages filtered by default configuration", "[timing][filter]")
{
  midi1_collector c;
  // Default: ignore_timing = true, ignore_sensing = true
  c.configuration.ignore_timing = true;
  c.configuration.ignore_sensing = true;
  auto sm = c.make_state_machine();

  SECTION("Clock, MTC, and SPP are filtered via on_bytes")
  {
    sm.on_bytes({std::vector<uint8_t>{0xF8}}, 0);       // Clock
    sm.on_bytes({std::vector<uint8_t>{0xF1, 0x00}}, 0); // MTC QF
    REQUIRE(c.messages.empty());
  }

  SECTION("Clock and MTC are filtered in on_bytes_multi")
  {
    const uint8_t bytes[] = {0xF8, 0xF1, 0x00};
    sm.on_bytes_multi(bytes, 0);
    REQUIRE(c.messages.empty());
  }

  SECTION("Active Sensing is filtered")
  {
    sm.on_bytes({std::vector<uint8_t>{0xFE}}, 0);
    REQUIRE(c.messages.empty());
  }

  SECTION("Start/Continue/Stop are NOT filtered (they are transport, not timing)")
  {
    sm.on_bytes({std::vector<uint8_t>{0xFA}}, 0); // Start
    sm.on_bytes({std::vector<uint8_t>{0xFB}}, 0); // Continue
    sm.on_bytes({std::vector<uint8_t>{0xFC}}, 0); // Stop
    REQUIRE(c.messages.size() == 3);
  }

  SECTION("Song Select and Tune Request are NOT filtered")
  {
    sm.on_bytes({std::vector<uint8_t>{0xF3, 0x00}}, 0); // Song Select
    sm.on_bytes({std::vector<uint8_t>{0xF6}}, 0);        // Tune Request
    REQUIRE(c.messages.size() == 2);
  }
}

// ============================================================================
// MIDI 1 <-> MIDI 2 (UMP) round-trip conversion for timing messages
// Uses the same midi1_to_midi2 / midi2_to_midi1 converters as midi_in/midi_out.
// ============================================================================

// Helper: convert MIDI 1 bytes to UMP, then back to MIDI 1, and verify identity
static void roundtrip_midi1_via_ump(std::span<const uint8_t> midi1_in)
{
  // MIDI 1 -> UMP
  libremidi::midi1_to_midi2 m1_to_m2;
  std::vector<uint32_t> ump_words;
  auto err1 = m1_to_m2.convert(midi1_in.data(), midi1_in.size(), 0, [&](auto* ump, auto n, auto) {
    ump_words.assign(ump, ump + n);
    return stdx::error{};
  });
  REQUIRE(err1 == stdx::error{});
  REQUIRE(!ump_words.empty());

  // UMP -> MIDI 1
  libremidi::midi2_to_midi1 m2_to_m1;
  std::vector<uint8_t> midi1_out;
  auto err2
      = m2_to_m1.convert(ump_words.data(), ump_words.size(), 0, [&](auto* midi, auto n, auto) {
          midi1_out.assign(midi, midi + n);
          return stdx::error{};
        });
  REQUIRE(err2 == stdx::error{});

  // Verify round-trip identity
  REQUIRE(midi1_out.size() == midi1_in.size());
  for (size_t i = 0; i < midi1_in.size(); ++i)
    REQUIRE(midi1_out[i] == midi1_in[i]);
}

TEST_CASE("MIDI 1 <-> UMP round-trip: MTC Quarter Frame", "[timing][midi2][mtc]")
{
  // Full MTC sequence: 8 quarter-frame messages for timecode 01:23:45:12 @ 25fps
  const uint8_t frames = 12, seconds = 45, minutes = 23, hours = 1;
  const uint8_t smpte_type = 1; // 25fps

  const uint8_t qf_messages[8][2] = {
      {0xF1, uint8_t(0x00 | (frames & 0x0F))},
      {0xF1, uint8_t(0x10 | (frames >> 4))},
      {0xF1, uint8_t(0x20 | (seconds & 0x0F))},
      {0xF1, uint8_t(0x30 | (seconds >> 4))},
      {0xF1, uint8_t(0x40 | (minutes & 0x0F))},
      {0xF1, uint8_t(0x50 | (minutes >> 4))},
      {0xF1, uint8_t(0x60 | (hours & 0x0F))},
      {0xF1, uint8_t(0x70 | (smpte_type << 1) | (hours >> 4))},
  };

  for (auto& qf : qf_messages)
    roundtrip_midi1_via_ump(qf);
}

TEST_CASE("MIDI 1 <-> UMP round-trip: Song Position Pointer", "[timing][midi2][spp]")
{
  for (int position : {0, 1, 96, 1000, 8191, 16383})
  {
    const uint8_t bytes[]
        = {0xF2, uint8_t(position & 0x7F), uint8_t((position >> 7) & 0x7F)};
    roundtrip_midi1_via_ump(bytes);
  }
}

TEST_CASE("MIDI 1 <-> UMP round-trip: Song Select", "[timing][midi2]")
{
  for (uint8_t song : {0, 1, 42, 127})
  {
    const uint8_t bytes[] = {0xF3, song};
    roundtrip_midi1_via_ump(bytes);
  }
}

TEST_CASE("MIDI 1 <-> UMP round-trip: real-time messages", "[timing][midi2]")
{
  // All single-byte system real-time messages
  SECTION("Timing Clock")
  {
    const uint8_t bytes[] = {0xF8};
    roundtrip_midi1_via_ump(bytes);
  }

  SECTION("Start")
  {
    const uint8_t bytes[] = {0xFA};
    roundtrip_midi1_via_ump(bytes);
  }

  SECTION("Continue")
  {
    const uint8_t bytes[] = {0xFB};
    roundtrip_midi1_via_ump(bytes);
  }

  SECTION("Stop")
  {
    const uint8_t bytes[] = {0xFC};
    roundtrip_midi1_via_ump(bytes);
  }

  SECTION("Active Sensing")
  {
    const uint8_t bytes[] = {0xFE};
    roundtrip_midi1_via_ump(bytes);
  }

  SECTION("System Reset")
  {
    const uint8_t bytes[] = {0xFF};
    roundtrip_midi1_via_ump(bytes);
  }

  SECTION("Tune Request")
  {
    const uint8_t bytes[] = {0xF6};
    roundtrip_midi1_via_ump(bytes);
  }
}

TEST_CASE("MIDI 1 -> UMP: verify system message UMP word layout", "[timing][midi2]")
{
  libremidi::midi1_to_midi2 m1_to_m2;

  auto convert_one = [&](std::span<const uint8_t> midi1) -> uint32_t {
    uint32_t result = 0;
    m1_to_m2.convert(midi1.data(), midi1.size(), 0, [&](auto* ump, auto n, auto) {
      REQUIRE(n == 1); // System messages are always 1 UMP word
      result = ump[0];
      return stdx::error{};
    });
    return result;
  };

  SECTION("Timing Clock -> UMP type 1, status 0xF8")
  {
    const uint8_t bytes[] = {0xF8};
    auto ump = convert_one(bytes);
    // UMP layout: [type=1 | group=0 | status=0xF8 | data=0 | data=0]
    REQUIRE(((ump >> 28) & 0xF) == 1);    // Message Type = System
    REQUIRE(((ump >> 16) & 0xFF) == 0xF8); // Status = Timing Clock
  }

  SECTION("MTC Quarter Frame -> UMP preserves data byte")
  {
    const uint8_t bytes[] = {0xF1, 0x47}; // type 4 (minutes low), value 7
    auto ump = convert_one(bytes);
    REQUIRE(((ump >> 16) & 0xFF) == 0xF1); // Status = MTC
    REQUIRE(((ump >> 8) & 0x7F) == 0x47);  // Data byte preserved
  }

  SECTION("Song Position Pointer -> UMP preserves both data bytes")
  {
    const uint8_t bytes[] = {0xF2, 0x10, 0x20}; // LSB=16, MSB=32 -> position=4112
    auto ump = convert_one(bytes);
    REQUIRE(((ump >> 16) & 0xFF) == 0xF2); // Status = SPP
    REQUIRE(((ump >> 8) & 0x7F) == 0x10);  // LSB
    REQUIRE((ump & 0x7F) == 0x20);          // MSB
  }
}

// Full pipeline test: MIDI 1 bytes -> midi1 state machine -> midi1_to_midi2 ->
// midi2 state machine -> midi2_to_midi1 -> verify original bytes
TEST_CASE("Full pipeline: MIDI 1 bytes -> decode -> UMP -> decode -> MIDI 1", "[timing][midi2]")
{
  // Step 1: decode MIDI 1 bytes through the midi1 state machine
  midi1_collector c;
  auto sm = c.make_state_machine();

  const uint8_t input[] = {
      0xFA,             // Start
      0xF8,             // Clock
      0xF1, 0x04,       // MTC QF (frame low nibble = 4)
      0xF2, 0x10, 0x20, // SPP (position = 4112)
      0xFC,             // Stop
  };
  sm.on_bytes_multi(input, 42);

  REQUIRE(c.messages.size() == 5);
  REQUIRE(c.messages[0][0] == 0xFA);
  REQUIRE(c.messages[1][0] == 0xF8);
  REQUIRE(c.messages[2][0] == 0xF1);
  REQUIRE(c.messages[3][0] == 0xF2);
  REQUIRE(c.messages[4][0] == 0xFC);

  // Step 2: convert each decoded MIDI 1 message to UMP
  libremidi::midi1_to_midi2 m1_to_m2;
  std::vector<libremidi::ump> umps;
  for (auto& msg : c.messages)
  {
    m1_to_m2.convert(
        msg.bytes.data(), msg.bytes.size(), msg.timestamp, [&](auto* ump, auto n, auto ts) {
          for (std::size_t i = 0; i < n;)
          {
            libremidi::ump u;
            auto words = cmidi2_ump_get_num_bytes(ump[i]) / 4;
            for (std::size_t w = 0; w < words && (i + w) < n; ++w)
              u.data[w] = ump[i + w];
            u.timestamp = ts;
            umps.push_back(u);
            i += words;
          }
          return stdx::error{};
        });
  }
  REQUIRE(umps.size() == 5);

  // Step 3: feed UMPs through the midi2 state machine
  libremidi::ump_input_configuration ump_conf;
  std::vector<libremidi::ump> decoded_umps;
  ump_conf.on_message = [&](libremidi::ump&& u) { decoded_umps.push_back(u); };
  ump_conf.timestamps = libremidi::timestamp_mode::NoTimestamp;
  ump_conf.ignore_timing = false;
  ump_conf.ignore_sensing = false;
  ump_conf.ignore_sysex = false;
  ump_conf.midi1_channel_events_to_midi2 = false;

  libremidi::midi2::input_state_machine sm2{ump_conf};
  for (auto& u : umps)
    sm2.on_bytes({u.data, u.data + u.size()}, 0);

  REQUIRE(decoded_umps.size() == 5);

  // Step 4: convert UMPs back to MIDI 1 and verify
  libremidi::midi2_to_midi1 m2_to_m1;
  std::vector<std::vector<uint8_t>> final_messages;
  for (auto& u : decoded_umps)
  {
    m2_to_m1.convert(u.data, u.size(), 0, [&](auto* midi, auto n, auto) {
      final_messages.emplace_back(midi, midi + n);
      return stdx::error{};
    });
  }

  REQUIRE(final_messages.size() == 5);

  // Verify each message matches the original
  for (size_t i = 0; i < c.messages.size(); ++i)
  {
    REQUIRE(final_messages[i].size() == c.messages[i].size());
    for (size_t j = 0; j < final_messages[i].size(); ++j)
      REQUIRE(final_messages[i][j] == c.messages[i][j]);
  }
}

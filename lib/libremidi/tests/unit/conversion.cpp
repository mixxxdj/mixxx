#include "../include_catch.hpp"

#include <libremidi/detail/conversion.hpp>
#include <libremidi/libremidi.hpp>

#include <span>
#include <iomanip>

namespace Catch
{
template <>
struct StringMaker<stdx::error>
{
  static std::string convert(stdx::error const& value)
  {
    const auto& msg = value.message();
    return std::string(msg.begin(), msg.end());
  }
};
}

static std::string hex_dump(const std::span<const uint8_t>& data)
{
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (auto b : data)
  {
    ss << std::setw(2) << static_cast<int>(b) << " ";
  }
  return ss.str();
}

TEST_CASE("Roundtrip MIDI 1 -> MIDI 2 -> MIDI 1", "[midi_roundtrip]")
{
  libremidi::midi1_to_midi2 m1_to_m2;
  libremidi::midi2_to_midi1 m2_to_m1;
  struct RoundTripTestCase
  {
    std::string name;
    std::vector<uint8_t> midi1_bytes;
  };
  std::vector<RoundTripTestCase> tests = {

      // // --- System Exclusive ---
      // 1. Short SysEx (fits in a single 64-bit UMP packet payload)
      // 0xF0 + ID + data + 0xF7. Total 6 bytes.
      {"SysEx Short", {0xF0, 0x7E, 0x00, 0x01, 0x02, 0xF7}},

      // 2. Long SysEx (Requires multiple UMP packets)
      // Payload > 6 bytes forces conversion to Start/Continue/End UMP packets.
      // F0 + 8 bytes data + F7 = 10 bytes total.
      {"SysEx Long", {0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xF7}},

      // --- System Common Messages ---
      {"Song Select", {0xF3, 0x05}},             // Song 5
      {"MTC Quarter Frame", {0xF1, 0x20}},       // Type 1, Value 0
      {"Song Position Ptr", {0xF2, 0x00, 0x01}}, // LSB 00, MSB 01
      // 0xF4, 0xF5 are undefined
      {"Tune Request", {0xF6}},

      // --- System Real-Time Messages ---
      {"Timing Clock", {0xF8}},
      {"Start", {0xFA}},
      {"Continue", {0xFB}},
      {"Stop", {0xFC}},
      // 0xF9, 0xFD are undefined
      {"Active Sensing", {0xFE}},
      {"Reset", {0xFF}},

      // --- Channel Voice Messages ---
      {"Note Off", {0x80, 0x3C, 0x00}},          // Ch 1, Note 60, Vel 0
      {"Note On", {0x90, 0x3C, 0x64}},           // Ch 1, Note 60, Vel 100
      {"Poly Key Pressure", {0xA1, 0x3C, 0x7F}}, // Ch 2, Note 60, Val 127
      {"Control Change", {0xB2, 0x07, 0x64}},    // Ch 3, CC 7, Val 100
      {"Program Change", {0xC3, 0x0A}},          // Ch 4, Prog 10
      {"Channel Pressure", {0xD4, 0x40}},        // Ch 5, Val 64
      {"Pitch Bend", {0xE5, 0x00, 0x40}},        // Ch 6, Center (LSB 00, MSB 40)

  };

  for (const auto& test : tests)
  {
    DYNAMIC_SECTION("Test Case: " << test.name)
    {

      // --- Step 1: MIDI 1 to MIDI 2 ---
      INFO("MIDI 1 -> MIDI 2");
      std::vector<uint32_t> ump_data;
      bool m2_ok = false;

      auto err1 = m1_to_m2.convert(
          test.midi1_bytes.data(), test.midi1_bytes.size(), 0,
          [&](uint32_t* ump, std::size_t sz, int64_t ts) {
        m2_ok = true;
        ump_data.insert(ump_data.end(), ump, ump + sz);
        return stdx::error{};
      });

      REQUIRE(err1 == stdx::error{});
      REQUIRE(m2_ok);
      REQUIRE(!ump_data.empty());

      INFO("MIDI 2 -> MIDI 1: success!");
      auto dmp
          = hex_dump(std::span<const uint8_t>((uint8_t*)ump_data.data(), ump_data.size() * 4));
      INFO(dmp);
      // --- Step 2: MIDI 2 to MIDI 1 ---
      INFO("MIDI 2 -> MIDI 1");
      std::vector<uint8_t> result_bytes;
      bool m1_ok = false;

      auto err2 = m2_to_m1.convert(
          ump_data.data(), ump_data.size(), 0, [&](uint8_t* midi, std::size_t sz, int64_t ts) {
        m1_ok = true;
        result_bytes.insert(result_bytes.end(), midi, midi + sz);
        return stdx::error{};
      });

      REQUIRE(err2 == stdx::error{});

      // Note: m1_ok might be false if the UMP was purely utility/metadata
      // and produced no MIDI 1 bytes, but for these test cases,
      // we expect output.
      REQUIRE(m1_ok);

      // --- Step 3: Verify Roundtrip ---

      // If the sizes differ, Catch2 will show the breakdown
      if (result_bytes.size() != test.midi1_bytes.size())
      {
        UNSCOPED_INFO(
            "Size mismatch! Expected " << test.midi1_bytes.size() << " but got "
                                       << result_bytes.size());
      }

      // Verify content
      bool content_match = (result_bytes == test.midi1_bytes);
      if (!content_match)
      {
        UNSCOPED_INFO("Input : " << hex_dump(test.midi1_bytes));
        UNSCOPED_INFO("Output: " << hex_dump(result_bytes));
      }

      REQUIRE(result_bytes == test.midi1_bytes);
    }
  }
}

TEST_CASE("convert midi1 to midi2", "[midi_in]")
{
  libremidi::midi1_to_midi2 m1;
  GIVEN("Correct input")
  {
    bool ok = false;
    auto msg = libremidi::channel_events::control_change(3, 35, 100);

    std::vector<uint32_t> res;
    auto err
        = m1.convert(msg.bytes.data(), msg.size(), 0, [&](uint32_t* ump, std::size_t sz, int64_t) {
      ok = true;
      res.assign(ump, ump + sz);
      return stdx::error{};
    });

    THEN("Correct conversion")
    {
      REQUIRE(err == stdx::error{});
      REQUIRE(ok);
      auto expected = cmidi2_ump_midi2_cc(0, 2, 35, 100 << 25);
      union
      {
        uint32_t u[2];
        int64_t i;
      } e{.i = expected};

      REQUIRE(res.size() == 2);
      REQUIRE(e.u[1] == res[0]);
      REQUIRE(e.u[0] == res[1]);
    }
  }

  GIVEN("Wrong input")
  {
    bool ok = false;
    unsigned char arr[1]{156};
    auto err = m1.convert(arr, 1, 0, [&](uint32_t*, std::size_t, int64_t) {
      ok = true;
      return stdx::error{};
    });
    THEN("We get an error")
    {
      REQUIRE(err != stdx::error{});
      REQUIRE(ok == false);
    }
  }
}

TEST_CASE("convert midi2 to midi1", "[midi_in]")
{
  libremidi::midi2_to_midi1 m1;
  GIVEN("Correct input")
  {
    bool ok = false;
    auto msg = cmidi2_ump_midi2_cc(0, 2, 35, 100 << 25);
    union
    {
      uint32_t u[2];
      int64_t i;
    } e{.i = msg};
    using namespace std;
    swap(e.u[0], e.u[1]);

    std::vector<uint8_t> res;
    auto err = m1.convert(e.u, 2, 0, [&](uint8_t* midi, std::size_t sz, int64_t) {
      ok = true;
      res.assign(midi, midi + sz);
      return stdx::error{};
    });

    THEN("Correct conversion")
    {
      REQUIRE(err == stdx::error{});
      REQUIRE(ok);
      auto msg = libremidi::channel_events::control_change(3, 35, 100);

      REQUIRE(res.size() == 3);
      REQUIRE(res == std::vector<uint8_t>(msg.begin(), msg.end()));
    }
  }

  GIVEN("Wrong input")
  {
    bool ok = false;
    uint32_t res = 0xFFFFFFFF;
    auto err = m1.convert(&res, 1, 0, [&](uint8_t*, std::size_t, int64_t) {
      ok = true;
      return stdx::error{};
    });
    THEN("We get an error")
    {
      REQUIRE(err != stdx::error{});
      REQUIRE(ok == false);
    }
  }
}

TEST_CASE("MIDI 1 MTC Quarter Frame Sequence -> MIDI 2 UMP", "[midi_conversion][mtc]")
{
  libremidi::midi1_to_midi2 m1_to_m2;

  // We will construct a sequence of 8 Quarter Frame messages
  // representing the timecode: 01:02:03:04 (HH:MM:SS:FF)
  //
  // Breakdown:
  // Frames  : 04 -> LSB=4 (Msg 0), MSB=0 (Msg 1)
  // Seconds : 03 -> LSB=3 (Msg 2), MSB=0 (Msg 3)
  // Minutes : 02 -> LSB=2 (Msg 4), MSB=0 (Msg 5)
  // Hours   : 01 -> LSB=1 (Msg 6), MSB=0 (Msg 7) (assuming 24fps/Type 0)

  std::vector<uint8_t> midi1_sequence = {
      0xF1, 0x04, // Piece 0: Frame LSB = 4
      0xF1, 0x10, // Piece 1: Frame MSB = 0
      0xF1, 0x23, // Piece 2: Seconds LSB = 3
      0xF1, 0x30, // Piece 3: Seconds MSB = 0
      0xF1, 0x42, // Piece 4: Minutes LSB = 2
      0xF1, 0x50, // Piece 5: Minutes MSB = 0
      0xF1, 0x61, // Piece 6: Hours LSB = 1
      0xF1, 0x70  // Piece 7: Hours MSB = 0
  };

  std::vector<uint32_t> expected_umps
      = {0x10F10400, // System Message (1), Group 0, Status F1, Data 04
         0x10F11000, // System Message (1), Group 0, Status F1, Data 10
         0x10F12300, // ...
         0x10F13000, 0x10F14200, 0x10F15000, 0x10F16100, 0x10F17000};

  std::vector<uint32_t> result_umps;
  bool ok = false;

  auto err = m1_to_m2.convert(
      midi1_sequence.data(), midi1_sequence.size(),
      0, // Group 0
      [&](uint32_t* ump, std::size_t sz, int64_t) {
    ok = true;
    result_umps.insert(result_umps.end(), ump, ump + sz);
    return stdx::error{};
  });

  REQUIRE(err == stdx::error{});
  REQUIRE(ok);

  SECTION("Verify UMP count")
  {
    // We sent 8 MIDI 1.0 messages, we expect 8 UMP packets (32-bit each)
    REQUIRE(result_umps.size() == 8);
  }

  SECTION("Verify UMP content")
  {
    for (size_t i = 0; i < expected_umps.size(); ++i)
    {
      INFO("Checking MTC Quarter Frame index " << i);

      // Breakdown for clearer error reporting if it fails
      uint32_t res = result_umps[i];
      uint32_t exp = expected_umps[i];

      // Verify the full UMP word matches the expectation
      REQUIRE(res == exp);

      // Redundant check: explicit field validation
      // UMP Type 1 (System Common), Group 0 -> High nibble 0x1
      REQUIRE((res >> 28) == 0x1);
      // Status Byte 0xF1
      REQUIRE(((res >> 16) & 0xFF) == 0xF1);
      // Data Byte (The MTC quarter frame payload)
      REQUIRE(((res >> 8) & 0xFF) == midi1_sequence[(i * 2) + 1]);
    }
  }
}

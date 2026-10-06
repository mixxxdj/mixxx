#include "../include_catch.hpp"

#include <libremidi/reader.hpp>
#include <libremidi/writer.hpp>

#include <filesystem>
TEST_CASE("write multiple tracks to file", "[midi_writer]")
{
  const uint8_t key1 = 60;
  const uint8_t key2 = 42;

  const std::string filename = "multitrack_test.mid";

  {
    libremidi::writer writer;

    const auto message1 = libremidi::channel_events::note_on(1, key1, 127);
    const auto message2 = libremidi::channel_events::note_on(1, key2, 127);

    writer.add_event(0, 0, message1);
    writer.add_event(0, 1, message2);

    std::ofstream file{filename, std::ios::binary | std::ios::trunc};
    writer.write(file);
  }

  {
    std::ifstream file{filename, std::ios::binary};
    std::vector<uint8_t> bytes;
    bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    libremidi::reader reader;
    libremidi::reader::parse_result result{};
    REQUIRE_NOTHROW(result = reader.parse(bytes));
    CHECK(result == libremidi::reader::validated);

    libremidi::message message1;
    libremidi::message message2;

    REQUIRE_NOTHROW(message1 = reader.tracks.at(0).at(0).m);
    REQUIRE_NOTHROW(message2 = reader.tracks.at(1).at(0).m);

    CHECK(message1.bytes[1] == 60);
    CHECK(message2.bytes[1] == 42);
  }
}

TEST_CASE("issue_54")
{
  // test out writer stuff.
  if (std::ofstream of("x.mid", std::ios::binary); of.good())
  {
    libremidi::message meta;
    libremidi::writer writer;

    // this block seems to create an unreadable file in any of the many tools tested?
    {
      // standard 4/4 timing
      meta = libremidi::meta_events::time_signature(4, 4);
      writer.add_event(0, libremidi::track_event{0, 0, meta});
      // 120 BPM as we know it Jim
      meta = libremidi::meta_events::tempo(50000);
      writer.add_event(0, libremidi::track_event{0, 0, meta});
    }

    // play middle C loudly
    libremidi::message msg = libremidi::channel_events::note_on(1, 60, 127);
    writer.add_event(0, libremidi::track_event{0, 0, msg});
    // off -why does it need velocity?
    msg = libremidi::channel_events::note_off(0, 60, 0);
    writer.add_event(0, libremidi::track_event{500, 0, msg});

    // does it need an end of track message?
    // not according to the library code
    meta = libremidi::meta_events::end_of_track();
    writer.add_event(0, libremidi::track_event{0, 0, meta});

    // write the content to file.
    writer.write(of);
  }

  {
    std::ifstream file{"x.mid", std::ios::binary};
    std::vector<uint8_t> bytes;
    bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    REQUIRE(bytes.size() > 0);
    libremidi::reader reader;
    libremidi::reader::parse_result result{};
    REQUIRE_NOTHROW(result = reader.parse(bytes));
    CHECK(result == libremidi::reader::validated);
  }
}

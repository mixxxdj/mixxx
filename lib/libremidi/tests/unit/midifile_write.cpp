#include "../include_catch.hpp"

#include <libremidi/writer.hpp>

#include <filesystem>

TEST_CASE("write an empty file", "[midi_writer]")
{
  libremidi::writer writer;
  std::ofstream empty;
  writer.write(empty);
}

TEST_CASE("write an empty track", "[midi_writer]")
{
  libremidi::writer writer;
  writer.tracks.resize(1);
  std::ofstream empty;
  writer.write(empty);
}

TEST_CASE("write multiple empty track", "[midi_writer]")
{
  libremidi::writer writer;
  writer.tracks.resize(2);
  std::ofstream empty;
  writer.write(empty);
}

TEST_CASE("write a track with an empty event", "[midi_writer]")
{
  libremidi::writer writer;
  writer.tracks.push_back(libremidi::midi_track{libremidi::track_event{}});
  std::ofstream empty;
  writer.write(empty);
}

TEST_CASE("write a track with a note", "[midi_writer]")
{
  libremidi::writer writer;
  writer.tracks.push_back(
      libremidi::midi_track{
          libremidi::track_event{0, 0, libremidi::channel_events::note_on(1, 45, 35)}});
  std::ofstream empty;
  writer.write(empty);
}

TEST_CASE("adding tracks", "[midi_writer]")
{
  libremidi::writer writer;
  writer.add_track();
  writer.add_track();
  writer.add_track();
}

TEST_CASE("adding events in tracks", "[midi_writer]")
{
  libremidi::writer writer;
  writer.add_event(4, {});
  writer.add_event(2, {});
  writer.add_event(7, {});
  writer.add_event(500, {});
}

TEST_CASE("adding events in invalid tracks", "[midi_writer]")
{
  libremidi::writer writer;
  REQUIRE_THROWS(writer.add_event(-1, {}));
  REQUIRE_THROWS(writer.add_event(500000, {}));
}

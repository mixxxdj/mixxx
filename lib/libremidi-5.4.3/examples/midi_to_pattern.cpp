#include <libremidi/reader.hpp>

#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <print>
#include <string>
#include <vector>

struct NoteEvent
{
  int64_t tick{};
  bool on{};
  friend bool operator<(NoteEvent lhs, NoteEvent rhs) noexcept { return lhs.tick < rhs.tick; }
  friend bool operator==(NoteEvent lhs, NoteEvent rhs) noexcept { return lhs.tick == rhs.tick; }
};

struct TrackData
{
  std::flat_map<int64_t, std::vector<NoteEvent>> events;
  int64_t max_tick = 0;
};

static int64_t array_gcd(const auto& arr)
{
  if (arr.empty())
    return 1;

  auto it = arr.begin();
  int64_t res = *it;
  ++it;

  for (; it != arr.end(); ++it)
    if ((res = std::gcd(*it, res)) == 1)
      break;

  return res;
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::println("Usage: {} <midi_file>", argv[0]);
    return 1;
  }

  // Load the midi file
  std::ifstream file{argv[1], std::ios::binary};
  if (!file)
  {
    std::println("Error: Could not open file '{}'", argv[1]);
    return 1;
  }

  std::vector<uint8_t> bytes;
  bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  file.close();

  libremidi::reader reader{true}; // Use absolute tick timing
  auto result = reader.parse(bytes);

  if (result == libremidi::reader::invalid)
  {
    std::println("Error: Invalid MIDI file");
    return 1;
  }

  // Process all tracks and collect note events
  TrackData processed_track;

  std::flat_set<int64_t> dates;
  for (const auto& track : reader.tracks)
  {
    for (const auto& event : track)
    {
      switch (event.m.get_message_type())
      {
        case libremidi::message_type::NOTE_ON: {
          int note = event.m[1];
          bool isOn = event.m[2] > 0;

          processed_track.events[note].push_back({event.tick, isOn});
          processed_track.max_tick = std::max(processed_track.max_tick, (int64_t)event.tick);
          dates.insert(event.tick);
          break;
        }
        case libremidi::message_type::NOTE_OFF: {
          int note = event.m[1];
          bool isOn = false;

          processed_track.events[note].push_back({event.tick, isOn});
          processed_track.max_tick = std::max(processed_track.max_tick, (int64_t)event.tick);
          break;
        }
        default:
          continue;
      }
    }
  }

  // Find GCD of each value
  dates.erase(0);
  const double gcd_in_64th_notes = 16. * (double(array_gcd(dates)) / reader.ticksPerBeat);

  const int ticks_per_division = gcd_in_64th_notes * reader.ticksPerBeat / 16;
  const int pattern_length = (processed_track.max_tick / ticks_per_division);

  // Generate pattern for each note
  for (const auto& [note, events] : processed_track.events)
  {
    std::string pattern;
    pattern.reserve(pattern_length);

    bool note_on = false;
    int event_index = 0;

    for (int pos = 0; pos < pattern_length; ++pos)
    {
      int tick = pos * ticks_per_division;

      // Process events at or before this position
      while (event_index < events.size() && events[event_index].tick <= tick)
      {
        note_on = events[event_index].on;
        event_index++;
      }

      if (note_on)
      {
        // Check if this is the start of a note or a continuation
        bool is_new_note = (pos == 0 || pattern.back() == '-' || pattern.back() == '0');
        pattern += is_new_note ? '1' : '2';
      }
      else
      {
        pattern += '-';
      }
    }

    // Only output if there's actual content
    if (!pattern.empty())
      std::print("{:02} {}\n", note, pattern);
  }

  return 0;
}

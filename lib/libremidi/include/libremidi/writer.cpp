/*
Copyright (c) 2015, Dimitri Diakopoulos All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if !defined(LIBREMIDI_HEADER_ONLY)
  #include <libremidi/writer.hpp>
#endif
#include <algorithm>
#include <bit>
#include <ostream>
#include <string>

#if !defined(__cpp_lib_bit_cast)
namespace std
{
template <typename R, typename T>
[[nodiscard]]
constexpr R bit_cast(const T& v) noexcept
{
  union
  {
    R res;
    T init;
  } u{.init = v};
  return u.res;
}
}
#endif
NAMESPACE_LIBREMIDI
{
namespace util
{
template <typename T>
LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::ostream& write_be(std::ostream& out, T value)
{
  static_assert(
      std::endian::native == std::endian::big || std::endian::native == std::endian::little);
  if constexpr (std::endian::native == std::endian::big)
  {
    out << value;
  }
  else
  {
    static constexpr auto n = sizeof(value);
    struct storage
    {
      uint8_t bytes[n];
    };
    auto data = std::bit_cast<storage>(value);
    std::reverse(data.bytes, data.bytes + n);
    out.write(reinterpret_cast<const char*>(data.bytes), static_cast<std::streamsize>(n));
  }
  return out;
}

// Write a number to the midifile
// as a variable length value which segments a file into 7-bit
// values.  Maximum size of aValue is 0x7fffffff
LIBREMIDI_STATIC_INLINE_IMPLEMENTATION void write_variable_length(uint32_t aValue, std::vector<uint8_t>& outdata)
{
  uint8_t bytes[5] = {0};

  bytes[0] = static_cast<uint8_t>((aValue >> 28) & 0x7F); // most significant 5 bits
  bytes[1] = static_cast<uint8_t>((aValue >> 21) & 0x7F); // next largest 7 bits
  bytes[2] = static_cast<uint8_t>((aValue >> 14) & 0x7F);
  bytes[3] = static_cast<uint8_t>((aValue >> 7) & 0x7F);
  bytes[4] = static_cast<uint8_t>((aValue) & 0x7F); // least significant 7 bits

  int start = 0;
  while (start < 5 && bytes[start] == 0)
    start++;

  for (int i = start; i < 4; i++)
  {
    bytes[i] = bytes[i] | 0x80;
    outdata.push_back(bytes[i]);
  }
  outdata.push_back(bytes[4]);
}

LIBREMIDI_STATIC_INLINE_IMPLEMENTATION void
add_event_track_count_check(std::vector<midi_track>& tracks, int track)
{
  if (track < 0)
    throw std::out_of_range("Refusing to add an event to track " + std::to_string(track) + ".");
  if (track > 65535)
    throw std::out_of_range(
        "Refusing to add an event to track " + std::to_string(track)
        + " ; change add_event_track_count_check in libremidi writer.cpp to increase the limit.");

  while (tracks.size() < static_cast<std::size_t>(track + 1)) // NOLINT(*-misplaced-widening-cast)
    tracks.emplace_back();
}
}

LIBREMIDI_INLINE
void writer::add_event(const int tick, const int track, const message& m)
{
  util::add_event_track_count_check(tracks, track);

  tracks[static_cast<std::size_t>(track)].push_back({tick, track, m});
}

LIBREMIDI_INLINE
void writer::add_event(int track, const track_event& m)
{
  util::add_event_track_count_check(tracks, track);

  tracks[static_cast<std::size_t>(track)].push_back(m);
}

LIBREMIDI_INLINE
void writer::add_track()
{
  util::add_event_track_count_check(tracks, static_cast<int>(tracks.size() + 1));
}

LIBREMIDI_INLINE
void writer::write(std::ostream& out) const
{
  // MIDI File Header
  out.write("MThd", 4);
  util::write_be<uint32_t>(out, 6);
  util::write_be<uint16_t>(out, (tracks.size() == 1) ? 0 : 1);
  util::write_be<uint16_t>(out, static_cast<uint16_t>(tracks.size()));
  util::write_be<uint16_t>(out, ticksPerQuarterNote);

  std::vector<uint8_t> track_raw_data;
  for (const auto& event_list : tracks)
  {
    track_raw_data.clear();
    // Rough estimation of the memory to allocate
    track_raw_data.reserve(event_list.size() * 3);

    for (const auto& event : event_list)
    {
      const auto& msg = event.m;
      if (msg.empty())
        continue;

      // Suppress end-of-track meta messages (one will be added
      // automatically after all track data has been written).
      if (msg.get_meta_event_type() == meta_event_type::END_OF_TRACK)
        continue;

      util::write_variable_length(static_cast<uint32_t>(event.tick), track_raw_data);

      if ((msg.get_message_type() == message_type::SYSTEM_EXCLUSIVE)
          || (event.m.get_message_type() == message_type::EOX))
      {
        // 0xf0 == Complete sysex message (0xf0 is part of the raw MIDI).
        // 0xf7 == Raw byte message (0xf7 not part of the raw MIDI).
        // Print the first byte of the message (0xf0 or 0xf7), then
        // print a VLV length for the rest of the bytes in the message.
        // In other words, when creating a 0xf0 or 0xf7 MIDI message,
        // do not insert the VLV byte length yourself, as this code will
        // do it for you automatically.
        track_raw_data.emplace_back(msg.bytes[0]); // 0xf0 or 0xf7;

        util::write_variable_length(static_cast<uint32_t>(msg.size()) - 1, track_raw_data);

        track_raw_data.insert(
            track_raw_data.end(), msg.bytes.data() + 1, msg.bytes.data() + msg.bytes.size());
      }
      else
      {
        // Non-sysex type of message, so just output the bytes of the message:
        track_raw_data.insert(
            track_raw_data.end(), msg.bytes.data(), msg.bytes.data() + msg.bytes.size());
      }
    }

    auto size = track_raw_data.size();
    const auto eot = meta_events::end_of_track();

    if ((size < 3) || !((track_raw_data[size - 3] == 0xFF) && (track_raw_data[size - 2] == 0x2F)))
    {
      track_raw_data.emplace_back(0x0); // tick
      track_raw_data.emplace_back(eot[0]);
      track_raw_data.emplace_back(eot[1]);
      track_raw_data.emplace_back(eot[2]);
    }

    // Write the track ID marker "MTrk":
    out.write("MTrk", 4);
    util::write_be<uint32_t>(out, static_cast<uint32_t>(track_raw_data.size()));
    out.write(
        reinterpret_cast<char*>(track_raw_data.data()),
        static_cast<std::streamsize>(track_raw_data.size()));
  }
}
}

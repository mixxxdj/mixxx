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
  #include <libremidi/reader.hpp>
#endif
#include <libremidi/message.hpp>

#include <algorithm>
#include <iostream>

// File Parsing Validation Todo:
// ==============================
// [] Bad file name
// [] Bad header
// [] Unknown header type
// [] Bad header size
// [] Bad type
// [] Bad tmecode
// [] Header too short
// [] Track too short
// [] Event too short
// ==============================

#if defined(__LIBREMIDI_DEBUG__)
std::ostream& operator<<(std::ostream& s, const libremidi::message& m)
{
  s << "[ MIDI: ";
  for (auto b : m)
    s << (unsigned int)b << ' ';
  s << "]\n";
  return s;
}
#endif

NAMESPACE_LIBREMIDI
{
namespace util
{
struct no_validator
{
  static bool validate_track([[maybe_unused]] const midi_track& track) { return true; }
};

struct validator
{
  static bool validate_track(const midi_track& track)
  {
    if (track.empty())
    {
#if defined(__LIBREMIDI_DEBUG__)
      std::cerr << "empty track" << std::endl;
#endif
      return false;
    }

    // Ensure that there is a unique EOT at the end of the track
    auto it = std::find_if(track.begin(), track.end(), [](const libremidi::track_event& msg) {
      static const auto eot = meta_events::end_of_track();
      return msg.m.bytes == eot.bytes;
    });

    if (it == track.end())
    {
#if defined(__LIBREMIDI_DEBUG__)
      std::cerr << "track has no END OF TRACK" << std::endl;
#endif
      return false;
    }

    if (&it->m != &track.back().m)
    {
#if defined(__LIBREMIDI_DEBUG__)
      std::cerr << std::distance(it, track.end());
      std::cerr << "track does not end with END OF TRACK" << std::endl;
#endif
      return false;
    }

    return true;
  }
};

// Used when we know that we have enough space
struct read_unchecked
{
  // Read a MIDI-style variable-length integer (big-endian value in groups of 7 bits,
  // with top bit set to signify that another byte follows).
  static void ensure_size(
      [[maybe_unused]] const uint8_t* begin, [[maybe_unused]] const uint8_t* end,
      [[maybe_unused]] int64_t needed)
  {
  }

  static uint32_t
  read_variable_length(uint8_t const*& data, [[maybe_unused]] uint8_t const* end)
  {
    uint32_t result = 0;
    while (true)
    {
      uint8_t b = *data++;
      if (b & 0x80)
      {
        result += (b & 0x7F);
        result <<= 7;
      }
      else
      {
        return result + b; // b is the last byte
      }
    }
  }

  static void read_bytes(
      midi_bytes& buffer, uint8_t const*& data, [[maybe_unused]] const uint8_t* end,
      const std::size_t num)
  {
    buffer.reserve(buffer.size() + num);
    for (std::size_t i = 0; i < num; ++i)
      buffer.push_back(*data++);
  }

  static void read_bytes(
      midi_bytes& buffer, uint8_t const*& data, [[maybe_unused]] const uint8_t* end, const int num)
  {
    read_bytes(buffer, data, end, static_cast<std::size_t>(num));
  }

  static uint16_t read_uint16_be(uint8_t const*& data, [[maybe_unused]] const uint8_t* end)
  {
    uint16_t result = *data++ << 8u;
    result += *data++;
    return result;
  }

  static uint32_t read_uint24_be(uint8_t const*& data, [[maybe_unused]] const uint8_t* end)
  {
    uint32_t result = *data++ << 16u;
    result += static_cast<uint32_t>(*data++ << 8u);
    result += *data++;
    return result;
  }

  static uint32_t read_uint32_be(uint8_t const*& data, [[maybe_unused]] const uint8_t* end)
  {
    uint32_t result = *data++ << 24u;
    result += static_cast<uint32_t>(*data++ << 16u);
    result += static_cast<uint32_t>(*data++ << 8u);
    result += *data++;
    return result;
  }
};

// Used when we do not know if we have enough bytes and have to check before reading
struct read_checked
{
  // Read a MIDI-style variable-length integer (big-endian value in groups of 7 bits,
  // with top bit set to signify that another byte follows).
  static void
  ensure_size(const uint8_t* begin, const uint8_t* end, const std::size_t needed)
  {
    if (const auto available = static_cast<std::size_t>(end - begin); available < needed)
      throw std::runtime_error("MIDI reader: not enough data to process");
  }

  static std::size_t read_variable_length(uint8_t const*& data, uint8_t const* end)
  {
    std::size_t result = 0;
    while (true)
    {
      ensure_size(data, end, 1);
      uint8_t b = *data++;
      if (b & 0x80)
      {
        result += (b & 0x7F);
        result <<= 7;
      }
      else
      {
        return result + b; // b is the last byte
      }
    }
  }

  static void
  read_bytes(midi_bytes& buffer, uint8_t const*& data, uint8_t const* end, const std::size_t num)
  {
    ensure_size(data, end, num);
    read_unchecked::read_bytes(buffer, data, end, num);
  }

  static uint16_t read_uint16_be(uint8_t const*& data, uint8_t const* end)
  {
    ensure_size(data, end, 2);
    return read_unchecked::read_uint16_be(data, end);
  }

  static uint32_t read_uint24_be(uint8_t const*& data, uint8_t const* end)
  {
    ensure_size(data, end, 3);
    return read_unchecked::read_uint24_be(data, end);
  }

  static uint32_t read_uint32_be(uint8_t const*& data, uint8_t const* end)
  {
    ensure_size(data, end, 4);
    return read_unchecked::read_uint32_be(data, end);
  }
};
}

#if defined(LIBREMIDI_UNCHECKED)
using byte_reader = util::read_unchecked;
#else
using byte_reader = util::read_checked;
#endif

#if defined(LIBREMIDI_UNVALIDATED)
using validator = util::no_validator;
#else
using validator = util::validator;
#endif

LIBREMIDI_INLINE
track_event parse_event(
    int tick, int track, const uint8_t*& dataStart, const uint8_t* dataEnd,
    message_type lastEventTypeByte)
{
  byte_reader::ensure_size(dataStart, dataEnd, 1);
  auto type = static_cast<message_type>(*dataStart++);

  track_event event{tick, track, message{}};

  if ((static_cast<uint8_t>(type) & 0xF0) == 0xF0)
  {
    // Meta event
    if (static_cast<uint8_t>(type) == 0xFF)
    {
      byte_reader::ensure_size(dataStart, dataEnd, 1);
      auto subtype = static_cast<meta_event_type>(*dataStart++);

      event.m.bytes.reserve(3);
      event.m.bytes.push_back(static_cast<uint8_t>(type));
      event.m.bytes.push_back(static_cast<uint8_t>(subtype));

      uint32_t length = 0;
      // Here we read the meta-event length manually, as this way we can also put it into
      // event.m.bytes
      while (true)
      {
        byte_reader::ensure_size(dataStart, dataEnd, 1);
        uint8_t b = *dataStart++;
        event.m.bytes.push_back(b);
        if (b & 0x80)
        {
          const uint8_t byte = (b & 0x7F);

          length += byte;
          length <<= 7;
        }
        else
        {
          length += b; // b is the last byte
          break;
        }
      }

      switch (subtype)
      {
        case meta_event_type::SEQUENCE_NUMBER: {
          switch (length)
          {
            case 0:
              return event;
            case 2:
              byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, 2);
              return event;
            default:
              throw std::invalid_argument("Expected length for SEQUENCE_NUMBER event is 0 or 2");
          }
        }
        case meta_event_type::TEXT:
        case meta_event_type::COPYRIGHT:
        case meta_event_type::TRACK_NAME:
        case meta_event_type::INSTRUMENT:
        case meta_event_type::LYRIC:
        case meta_event_type::MARKER:
        case meta_event_type::CUE:
        case meta_event_type::PATCH_NAME:
        case meta_event_type::DEVICE_NAME: {
          byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
          return event;
        }

        case meta_event_type::END_OF_TRACK: {
          if (length != 0)
            throw std::invalid_argument("Expected length for END_OF_TRACK event is 0");
          return event;
        }
        case meta_event_type::TEMPO_CHANGE: {
          if (length != 3)
            throw std::invalid_argument("Expected length for TEMPO_CHANGE event is 3");
          // event.m.bytes[3] = read_uint24_be(dataStart); // @dimitri TOFIX
          byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
          return event;
        }
        case meta_event_type::SMPTE_OFFSET: {
          if (length != 5)
            throw std::invalid_argument("Expected length for SMPTE_OFFSET event is 5");
          byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
          auto& b = event.m.bytes;

          uint8_t format = (b[3] & 0b01100000) >> 5;
          uint8_t h = (b[3] & 0b00011111);

          if (format > 3)
            throw std::invalid_argument("SMPTE_OFFSET has unknown format");

          int max = 0;
          switch (format)
          {
            case 0: // 24
              max = 24;
              break;
            case 1: // 25
              max = 25;
              break;
            case 2: // 29
              max = 29;
              break;
            case 3: // 30
              max = 30;
              break;
            default:
              break;
          }

          if (h >= 24 || b[4] >= 60 || b[5] >= 60 || b[6] >= max || b[7] >= 100)
            throw std::invalid_argument("SMPTE_OFFSET is out-of-23:59:59:xx:99 bounds");
          return event;
        }
        case meta_event_type::TIME_SIGNATURE: {
          if (length != 4)
            throw std::invalid_argument("Expected length for TIME_SIGNATURE event is 4");
          byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
          return event;
        }
        case meta_event_type::KEY_SIGNATURE: {
          if (length != 2)
            throw std::invalid_argument("Expected length for KEY_SIGNATURE event is 2");
          byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
          auto k = static_cast<int8_t>(event.m[3]);
          if (k < -7 || k > 7)
            throw std::invalid_argument("Invalid KEY_SIGNATURE");
          if (event.m[4] > 1)
            throw std::invalid_argument("Invalid KEY_SIGNATURE");
          return event;
        }
        case meta_event_type::PROPRIETARY: {
          byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
          return event;
        }
        case meta_event_type::CHANNEL_PREFIX: {
          if (length != 1)
            throw std::invalid_argument("Expected length for CHANNEL_PREFIX event is 1");
          byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
          return event;
        }
        case meta_event_type::MIDI_PORT: {
          if (length != 1)
            throw std::invalid_argument("Expected length for MIDI_PORT event is 1");
          byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
          return event;
        }
        case meta_event_type::UNKNOWN:
        default: {
          // Unknown events?
          byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
          return event;
        }
      }
    }

    else if (type == message_type::SYSTEM_EXCLUSIVE)
    {
      const auto length = byte_reader::read_variable_length(dataStart, dataEnd);
      event.m.bytes = {static_cast<uint8_t>(type)};
      byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
      return event;
    }

    else if (type == message_type::EOX)
    {
      const auto length = byte_reader::read_variable_length(dataStart, dataEnd);
      byte_reader::read_bytes(event.m.bytes, dataStart, dataEnd, length);
      return event;
    }
    else
    {
      throw std::runtime_error("Unrecognised MIDI event type byte");
    }
  }

  // Channel events
  else
  {
    event.m.bytes.clear();

    // Running status...
    if ((static_cast<uint8_t>(type) & 0x80) == 0)
    {
      // Reuse lastEventTypeByte as the event type.
      // eventTypeByte is actually the first parameter
      event.m.bytes.push_back(static_cast<uint8_t>(lastEventTypeByte));
      event.m.bytes.push_back(static_cast<uint8_t>(type));
      type = lastEventTypeByte;
    }
    else
    {
      event.m.bytes.push_back(static_cast<uint8_t>(type));

      byte_reader::ensure_size(dataStart, dataEnd, 1);
      event.m.bytes.push_back(static_cast<uint8_t>(*dataStart++));
      lastEventTypeByte = type;
    }

    static constexpr auto validate = [](midi_bytes& b) {
      if (b[1] < 128 && b[2] < 128)
        return true;
      throw std::invalid_argument("MIDI message has arguments > 127");
    };

    switch (static_cast<message_type>(static_cast<uint8_t>(type) & 0xF0))
    {
      case message_type::NOTE_OFF:
      case message_type::NOTE_ON:
      case message_type::POLY_PRESSURE:
      case message_type::CONTROL_CHANGE:
        byte_reader::ensure_size(dataStart, dataEnd, 1);
        event.m.bytes.push_back(*dataStart++);
        validate(event.m.bytes);
        return event;
      case message_type::PROGRAM_CHANGE:
        if (event.m.bytes[1] >= 128)
          throw std::invalid_argument("MIDI PC has arguments > 127");
        return event;
      case message_type::AFTERTOUCH:
        if (event.m.bytes[1] >= 128)
          throw std::invalid_argument("MIDI Atertouch has arguments > 127");
        return event;
      case message_type::PITCH_BEND:
        byte_reader::ensure_size(dataStart, dataEnd, 1);
        event.m.bytes.push_back(*dataStart++);
        validate(event.m.bytes);
        return event;

      case message_type::TIME_CODE:
        throw std::runtime_error("Unsupported MIDI event type TIME_CODE");
      case message_type::SONG_POS_POINTER:
        throw std::runtime_error("Unsupported MIDI event type SONG_POS_POINTER");
      case message_type::SONG_SELECT:
        throw std::runtime_error("Unsupported MIDI event type SONG_SELECT");
      case message_type::RESERVED1:
        throw std::runtime_error("Unsupported MIDI event type RESERVED1");
      case message_type::RESERVED2:
        throw std::runtime_error("Unsupported MIDI event type RESERVED2");
      case message_type::TUNE_REQUEST:
        throw std::runtime_error("Unsupported MIDI event type TUNE_REQUEST");
      case message_type::EOX:
        throw std::runtime_error("Unsupported MIDI event type EOX");
        // System Realtime Messages :
      case message_type::TIME_CLOCK:
        throw std::runtime_error("Unsupported MIDI event type TIME_CLOCK");
      case message_type::RESERVED3:
        throw std::runtime_error("Unsupported MIDI event type RESERVED3");
      case message_type::START:
        throw std::runtime_error("Unsupported MIDI event type START");
      case message_type::CONTINUE:
        throw std::runtime_error("Unsupported MIDI event type CONTINUE");
      case message_type::STOP:
        throw std::runtime_error("Unsupported MIDI event type STOP");
      case message_type::RESERVED4:
        throw std::runtime_error("Unsupported MIDI event type RESERVED4");
      case message_type::ACTIVE_SENSING:
        throw std::runtime_error("Unsupported MIDI event type ACTIVE_SENSING");
      case message_type::SYSTEM_RESET:
        throw std::runtime_error("Unsupported MIDI event type SYSTEM_RESET");
      case message_type::INVALID:
        throw std::runtime_error("Unsupported MIDI event type INVALID");
      case message_type::SYSTEM_EXCLUSIVE:
        throw std::runtime_error("Unsupported MIDI event type SYSTEM_EXCLUSIVE");
      default:
        throw std::runtime_error("Unsupported MIDI event type");
    }
  }
}

LIBREMIDI_INLINE
reader::reader(bool useAbsolute)
    : ticksPerBeat(480)
    , startingTempo(120)
    , m_useAbsoluteTicks(useAbsolute)
{
}

LIBREMIDI_INLINE
reader::~reader() { }

constexpr int str_to_headerid(const char* str)
{
  return str[0] << 24 | str[1] << 16 | str[2] << 8 | str[3];
}

LIBREMIDI_INLINE
auto reader::parse(const uint8_t* dataPtr, std::size_t size) noexcept -> parse_result
try
{
  using namespace util;

  tracks.clear();

  if (size == 0)
  {
#if defined(__LIBREMIDI_DEBUG__)
    std::cerr << "empty buffer passed to parse." << std::endl;
#endif
    return parse_result::invalid;
  }

  const uint8_t* const data_end = dataPtr + size;

  uint32_t header_id = read_checked::read_uint32_be(dataPtr, data_end);
  uint32_t header_length = read_checked::read_uint32_be(dataPtr, data_end);

  if (static_cast<int>(header_id) != str_to_headerid("MThd") || header_length != 6)
  {
#if defined(__LIBREMIDI_DEBUG__)
    std::cerr << "couldn't parse header" << std::endl;
#endif
    return parse_result::invalid;
  }

  format = read_checked::read_uint16_be(
      dataPtr, data_end); //@tofix format type -> save for later eventually
  if (format > 2)
  {
#if defined(__LIBREMIDI_DEBUG__)
    std::cerr << "unknown format" << std::endl;
#endif
    return parse_result::invalid;
  }
  int track_count = read_checked::read_uint16_be(dataPtr, data_end);
  uint16_t time_division = read_checked::read_uint16_be(dataPtr, data_end);

  // CBB: deal with the SMPTE style time coding
  // timeDivision is described here http://www.sonicspot.com/guide/midifiles.html
  if (time_division & 0x8000)
  {
#if defined(__LIBREMIDI_DEBUG__)
    std::cerr << "found SMPTE time frames (unsupported)" << std::endl;
    int fps = (timeDivision >> 16) & 0x7f;
    if (fps != -30 && fps != -29 && fps != -25 && fps != -24)
      return parse_result::invalid;
    int ticksPerFrame = timeDivision & 0xff;
#endif
    // given beats per second, timeDivision should be derivable.
    return parse_result::invalid;
  }

  startingTempo = 120.0f;              // midi default
  ticksPerBeat = float(time_division); // ticks per beat (a beat is defined as a quarter note)

  parse_result result = parse_result::validated;

  for (int i = 0; i < track_count; ++i)
  {
    midi_track track;

    header_id = read_checked::read_uint32_be(dataPtr, data_end);
    header_length = read_checked::read_uint32_be(dataPtr, data_end);

    if (header_id != str_to_headerid("MTrk"))
    {
#if defined(__LIBREMIDI_DEBUG__)
      std::cerr << "couldn't find track header" << std::endl;
#endif
      return parse_result::incomplete;
    }

    int64_t available = data_end - dataPtr;
    if (available < header_length)
    {
#if defined(__LIBREMIDI_DEBUG__)
      std::cerr << "not enough data available" << std::endl;
#endif
      return parse_result::incomplete;
    }

    track.reserve(header_length / 3);

    const uint8_t* const track_end = dataPtr + header_length;

    auto running_event = message_type::INVALID;

    std::size_t tick_count = 0;

    while (dataPtr < track_end)
    {
      const auto tick = read_checked::read_variable_length(dataPtr, track_end);
      if (m_useAbsoluteTicks)
      {
        tick_count += tick;
      }
      else
      {
        tick_count = tick;
      }

      try
      {
        track_event ev
            = parse_event(static_cast<int>(tick_count), i, dataPtr, track_end, running_event);
        if (!ev.m.empty())
        {
          if (!ev.m.is_meta_event())
          {
            running_event = static_cast<message_type>(ev.m.bytes[0]);
          }
        }
        else
        {
#if defined(__LIBREMIDI_DEBUG__)
          std::cerr << "could not read event" << std::endl;
#endif
          dataPtr = track_end;
          result = parse_result::incomplete;
          continue;
        }

        track.push_back(std::move(ev));
      }
      catch (const std::exception& e)
      {
#if defined(__LIBREMIDI_DEBUG__)
        std::cerr << "" << e.what() << std::endl;
#endif
        dataPtr = track_end;
        result = parse_result::incomplete;
        continue;
      }
    }

    if (result == parse_result::validated)
    {
      if (!validator::validate_track(track))
      {
        result = parse_result::complete;
      }
    }
    tracks.push_back(std::move(track));
  }

  if (result == parse_result::validated)
  {
    if (dataPtr != data_end)
    {
#if defined(__LIBREMIDI_DEBUG__)
      std::cerr << "midifile has junk at end: " << std::intptr_t(dataEnd - dataPtr) << std::endl;
#endif
      result = parse_result::complete;
    }
  }
  return result;
}
catch (const std::exception& e)
{
#if defined(__LIBREMIDI_DEBUG__)
  std::cerr << "" << e.what() << std::endl;
#endif
  return parse_result::invalid;
}

// In ticks
LIBREMIDI_INLINE
double reader::get_end_time() const noexcept
{
  if (m_useAbsoluteTicks)
  {
    double total_length = 0.;
    for (const auto& t : tracks)
    {
      if (!t.empty())
      {
        const auto& last_event = t.back();
        if (last_event.tick > total_length)
          total_length = last_event.tick;
      }
    }
    return total_length;
  }
  else
  {
    double total_length = 0.;
    for (const auto& t : tracks)
    {
      double track_length = 0.;
      for (const auto& e : t)
        track_length += e.tick;

      if (track_length > total_length)
        total_length = track_length;
    }
    return total_length;
  }
}

LIBREMIDI_INLINE
auto reader::parse(const std::vector<uint8_t>& buffer) noexcept -> parse_result
{
  return parse(buffer.data(), buffer.size());
}

#if defined(LIBREMIDI_HAS_SPAN)
LIBREMIDI_INLINE
auto reader::parse(std::span<uint8_t> buffer) noexcept -> parse_result
{
  return parse(buffer.data(), buffer.size());
}
#endif
}

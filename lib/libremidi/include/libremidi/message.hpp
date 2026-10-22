#pragma once
#include <libremidi/config.hpp>

#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>
#if defined(__cpp_exceptions)
  #include <stdexcept>
#endif

NAMESPACE_LIBREMIDI
{
enum class message_type : uint8_t
{
  INVALID = 0x0,
  // Standard Message
  NOTE_OFF = 0x80,
  NOTE_ON = 0x90,
  POLY_PRESSURE = 0xA0,
  CONTROL_CHANGE = 0xB0,
  PROGRAM_CHANGE = 0xC0,
  AFTERTOUCH = 0xD0,
  PITCH_BEND = 0xE0,

  // System Common Messages
  SYSTEM_EXCLUSIVE = 0xF0,
  TIME_CODE = 0xF1,
  SONG_POS_POINTER = 0xF2,
  SONG_SELECT = 0xF3,
  RESERVED1 = 0xF4,
  RESERVED2 = 0xF5,
  TUNE_REQUEST = 0xF6,
  EOX = 0xF7,

  // System Realtime Messages
  TIME_CLOCK = 0xF8,
  RESERVED3 = 0xF9,
  START = 0xFA,
  CONTINUE = 0xFB,
  STOP = 0xFC,
  RESERVED4 = 0xFD,
  ACTIVE_SENSING = 0xFE,
  SYSTEM_RESET = 0xFF
};

enum class meta_event_type : uint8_t
{
  SEQUENCE_NUMBER = 0x00,
  TEXT = 0x01,
  COPYRIGHT = 0x02,
  TRACK_NAME = 0x03,
  INSTRUMENT = 0x04,
  LYRIC = 0x05,
  MARKER = 0x06,
  CUE = 0x07,
  PATCH_NAME = 0x08,
  DEVICE_NAME = 0x09,
  CHANNEL_PREFIX = 0x20,
  MIDI_PORT = 0x21,
  END_OF_TRACK = 0x2F,
  TEMPO_CHANGE = 0x51,
  SMPTE_OFFSET = 0x54,
  TIME_SIGNATURE = 0x58,
  KEY_SIGNATURE = 0x59,
  PROPRIETARY = 0x7F,
  UNKNOWN = 0xFF
};

struct message
{
  midi_bytes bytes;
  int64_t timestamp{};

  message() noexcept = default;
  operator std::span<const unsigned char>() const noexcept { return {bytes.data(), bytes.size()}; }

  message(const midi_bytes& src_bytes, int64_t src_timestamp) noexcept
      : bytes(src_bytes)
      , timestamp(src_timestamp)
  {
  }
  message(std::initializer_list<unsigned char> args) noexcept
      : bytes{args}
  {
  }

  template <typename... Args>
  auto assign(Args&&... args)
  {
    return bytes.assign(std::forward<Args>(args)...);
  }
  template <typename... Args>
  auto insert(Args&&... args)
  {
    return bytes.insert(std::forward<Args>(args)...);
  }

  auto size() const noexcept { return bytes.size(); }
  auto empty() const noexcept { return bytes.empty(); }

  auto clear() noexcept { bytes.clear(); }

  auto& operator[](midi_bytes::size_type i) const noexcept { return bytes[i]; }
  auto& operator[](midi_bytes::size_type i) noexcept { return bytes[i]; }

  auto& front() const { return bytes.front(); }
  auto& back() const { return bytes.back(); }
  auto& front() { return bytes.front(); }
  auto& back() { return bytes.back(); }

  auto begin() const noexcept { return bytes.begin(); }
  auto end() const noexcept { return bytes.end(); }
  auto begin() noexcept { return bytes.begin(); }
  auto end() noexcept { return bytes.end(); }
  auto cbegin() const noexcept { return bytes.cbegin(); }
  auto cend() const noexcept { return bytes.cend(); }
  auto cbegin() noexcept { return bytes.cbegin(); }
  auto cend() noexcept { return bytes.cend(); }
  auto rbegin() const noexcept { return bytes.rbegin(); }
  auto rend() const noexcept { return bytes.rend(); }
  auto rbegin() noexcept { return bytes.rbegin(); }
  auto rend() noexcept { return bytes.rend(); }

  bool uses_channel(int channel) const LIBREMIDI_PRECONDITION(channel > 0 && channel <= 16)
  {
    return ((bytes[0] & 0xF) == channel - 1) && ((bytes[0] & 0xF0) != 0xF0);
  }

  int get_channel() const noexcept
  {
    if ((bytes[0] & 0xF0) != 0xF0)
      return (bytes[0] & 0xF) + 1;
    return 0;
  }

  bool is_meta_event() const noexcept { return bytes[0] == 0xFF; }

  meta_event_type get_meta_event_type() const noexcept
  {
    if (!is_meta_event())
      return meta_event_type::UNKNOWN;
    return static_cast<meta_event_type>(bytes[1]);
  }

  message_type get_message_type() const noexcept
  {
    if (bytes[0] >= static_cast<uint8_t>(message_type::SYSTEM_EXCLUSIVE))
    {
      return static_cast<message_type>(bytes[0] & 0xFF);
    }
    else
    {
      return static_cast<message_type>(bytes[0] & 0xF0);
    }
  }

  bool is_note_on_or_off() const noexcept
  {
    const auto status = get_message_type();
    return (status == message_type::NOTE_ON) || (status == message_type::NOTE_OFF);
  }
};

struct channel_events
{
  static constexpr uint8_t clamp_channel(int channel) noexcept
  {
    channel--;
    if (channel < 0)
      channel = 0;
    else if (channel > 15)
      channel = 15;
    return static_cast<uint8_t>(channel);
  }

  static uint8_t make_command(const message_type type, const int channel) noexcept
  {
    return static_cast<uint8_t>(static_cast<uint8_t>(type) | clamp_channel(channel));
  }

  static message note_on(uint8_t channel, uint8_t note, uint8_t velocity) noexcept
  {
    return {make_command(message_type::NOTE_ON, channel), note, velocity};
  }

  static message note_off(uint8_t channel, uint8_t note, uint8_t velocity) noexcept
  {
    return {make_command(message_type::NOTE_OFF, channel), note, velocity};
  }

  static message control_change(uint8_t channel, uint8_t control, uint8_t value) noexcept
  {
    return {make_command(message_type::CONTROL_CHANGE, channel), control, value};
  }

  static message program_change(uint8_t channel, uint8_t value) noexcept
  {
    return {make_command(message_type::PROGRAM_CHANGE, channel), value};
  }

  static message pitch_bend(uint8_t channel, int value) noexcept
  {
    return {
        make_command(message_type::PITCH_BEND, channel), static_cast<unsigned char>(value & 0x7F),
        static_cast<uint8_t>((value >> 7) & 0x7F)};
  }

  static message pitch_bend(uint8_t channel, uint8_t lsb, uint8_t msb) noexcept
  {
    return {make_command(message_type::PITCH_BEND, channel), lsb, msb};
  }

  static message poly_pressure(uint8_t channel, uint8_t note, uint8_t value) noexcept
  {
    return {make_command(message_type::POLY_PRESSURE, channel), note, value};
  }

  static message aftertouch(uint8_t channel, uint8_t value) noexcept
  {
    return {make_command(message_type::AFTERTOUCH, channel), value};
  }
};

struct meta_events
{
  static message end_of_track() noexcept { return {0xFF, 0x2F, 0}; }

  static message channel(int channel) noexcept
  {
    return {0xff, 0x20, 0x01, static_cast<uint8_t>(std::clamp(0, 0xff, channel - 1))};
  }

  static message tempo(int mpqn) noexcept
  {
    return {
        0xff,
        81,
        3,
        static_cast<uint8_t>(mpqn >> 16),
        static_cast<uint8_t>(mpqn >> 8),
        static_cast<uint8_t>(mpqn)};
  }

  static message time_signature(
      int numerator, int denominator, int clocks_per_click = 1,
      int notated_32nd_notes_per_beat = 96)
  {
    int n = 1;
    int pow_two = 0;

    while (n < denominator)
    {
      n <<= 1;
      ++pow_two;
    }

    return {
        0xff,
        0x58,
        0x04,
        static_cast<uint8_t>(numerator),
        static_cast<uint8_t>(pow_two),
        static_cast<uint8_t>(clocks_per_click),
        static_cast<uint8_t>(notated_32nd_notes_per_beat)};
  }

  // Where key index goes from -7 (7 flats, C♭ Major) to +7 (7 sharps, C♯
  // Major)
  static message key_signature(int keyIndex, bool isMinor)
  {
#if defined(__cpp_exceptions)
    if (keyIndex < -7 || keyIndex > 7)
      throw std::range_error("out of range");
#endif
    return {
        0xff, 0x59, 0x02, static_cast<uint8_t>(keyIndex),
        isMinor ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0)};
  }

  static message song_position(int positionInBeats) noexcept
  {
    return {
        0xf2, static_cast<uint8_t>(positionInBeats & 127),
        static_cast<uint8_t>((positionInBeats >> 7) & 127)};
  }
};

struct track_event
{
  int tick = 0;
  int track = 0;
  message m;
};

typedef std::vector<track_event> midi_track;
}

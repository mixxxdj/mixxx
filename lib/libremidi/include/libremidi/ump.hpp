#pragma once
#include <libremidi/config.hpp>

#if LIBREMIDI_NI_MIDI2_COMPAT
  #include <midi/universal_packet.h>
#endif

NAMESPACE_LIBREMIDI
{
namespace midi2
{
// Imported from cmidi2
enum class message_type
{
  // MIDI 2.0 UMP Section 3.
  UTILITY = 0,
  SYSTEM = 1,
  MIDI_1_CHANNEL = 2,
  SYSEX7 = 3,
  MIDI_2_CHANNEL = 4,
  SYSEX8_MDS = 5,
};
}

struct ump
{
  alignas(4) uint32_t data[4] = {};
  int64_t timestamp{};

  constexpr ump() noexcept = default;
  constexpr ~ump() = default;

  explicit constexpr ump(uint32_t b0) noexcept
      : data{b0, 0, 0, 0}
  {
  }
  constexpr ump(uint32_t b0, uint32_t b1) noexcept
      : data{b0, b1, 0, 0}
  {
  }
  constexpr ump(uint32_t b0, uint32_t b1, uint32_t b2) noexcept
      : data{b0, b1, b2, 0}
  {
  }
  constexpr ump(uint32_t b0, uint32_t b1, uint32_t b2, uint32_t b3) noexcept
      : data{b0, b1, b2, b3}
  {
  }

  // Compatibility with ni-midi2:
#if LIBREMIDI_NI_MIDI2_COMPAT
  constexpr operator midi::universal_packet() const noexcept
  {
    return {data[0], data[1], data[2], data[3]};
  }
  explicit constexpr ump(midi::universal_packet b) noexcept
      : data{b.data[0], b.data[1], b.data[2], b.data[3]}
  {
  }
  constexpr ump& operator=(midi::universal_packet b) noexcept
  {
    data[0] = b.data[0];
    data[1] = b.data[1];
    data[2] = b.data[2];
    data[3] = b.data[3];
    return *this;
  }
#endif

  // Compatibility with cmidi2:
  operator uint32_t*() & noexcept { return data; }
  operator const uint32_t*() const& noexcept { return data; }
  operator uint32_t*() && noexcept = delete;
  operator const uint32_t*() const&& noexcept = delete;

  constexpr std::size_t size() const noexcept
  {
    switch (midi2::message_type(((data[0] & 0xF0000000) >> 28) & 0xF))
    {
      case midi2::message_type::UTILITY:
      case midi2::message_type::SYSTEM:
      case midi2::message_type::MIDI_1_CHANNEL:
        return 1;
      case midi2::message_type::MIDI_2_CHANNEL:
      case midi2::message_type::SYSEX7:
        return 2;
      case midi2::message_type::SYSEX8_MDS:
        return 4;
      default:
        return 0;
    }
  }

  constexpr void clear() noexcept
  {
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
  }

  constexpr auto& operator[](int i) const noexcept { return data[i]; }
  constexpr auto& operator[](int i) noexcept { return data[i]; }

  constexpr auto begin() const noexcept { return data; }
  constexpr auto end() const noexcept { return data + size(); }
  constexpr auto begin() noexcept { return data; }
  constexpr auto end() noexcept { return data + size(); }
  constexpr auto cbegin() const noexcept { return data; }
  constexpr auto cend() const noexcept { return data + size(); }
  constexpr auto cbegin() noexcept { return data; }
  constexpr auto cend() noexcept { return data + size(); }

  constexpr midi2::message_type get_type() const noexcept
  {
    return midi2::message_type(((data[0] & 0xF0000000) >> 28) & 0xF);
  }
  constexpr uint8_t get_group() const noexcept { return (data[0] >> 24) & 0x0F; }
  constexpr uint8_t get_channel_0_15() const noexcept { return (data[0] >> 16) & 0x0F; }
  constexpr uint8_t get_channel() const noexcept { return get_channel_0_15() + 1; }
  constexpr uint8_t get_status_code() const noexcept { return (data[0] >> 16) & 0xF0; }
};
}

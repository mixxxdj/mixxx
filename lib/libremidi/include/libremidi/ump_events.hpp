#pragma once
#include <libremidi/cmidi2.hpp>
#include <libremidi/detail/conversion.hpp>
#include <libremidi/ump.hpp>

// FIXME! we need to review them all to check the channels
NAMESPACE_LIBREMIDI::ump_events
{
inline libremidi::ump note_on(uint8_t group, uint8_t channel, uint8_t pitch, uint32_t vel)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_note_on(group, channel, pitch, 0, vel, 0);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump note_off(uint8_t group, uint8_t channel, uint8_t pitch, uint32_t vel)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_note_off(group, channel, pitch, 0, vel, 0);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump poly_pressure(uint8_t group, uint8_t channel, uint8_t pitch, uint32_t vel)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_paf(group, channel, pitch, vel);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump control_change(uint8_t group, uint8_t channel, uint8_t index, uint32_t value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_cc(group, channel, index, value);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump program_change(uint8_t group, uint8_t channel, uint8_t index)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_program(group, channel, 0, index, 0, 0);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump aftertouch(uint8_t group, uint8_t channel, uint32_t value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_caf(group, channel, value);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump pitch_bend(uint8_t group, uint8_t channel, uint32_t value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_pitch_bend_direct(group, channel, value);
  cmidi2_reverse(msg, u.data);
  return u;
}
}

NAMESPACE_LIBREMIDI::from_midi1
{
inline libremidi::ump note_on(uint8_t channel, uint8_t pitch, uint8_t vel)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_note_on(0, channel, pitch, 0, u7_to_u16(vel), 0);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump note_off(uint8_t channel, uint8_t pitch, uint8_t vel)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_note_off(0, channel, pitch, 0, u7_to_u16(vel), 0);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump poly_pressure(uint8_t channel, uint8_t pitch, uint8_t value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_paf(0, channel, pitch, u7_to_u32(value));
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump control_change(uint8_t channel, uint8_t index, uint8_t value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_cc(0, channel, index, u7_to_u32(value));
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump program_change(uint8_t channel, uint8_t index)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_program(0, channel, 0, index, 0, 0);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump aftertouch(uint8_t channel, uint8_t value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_caf(0, channel, u7_to_u32(value));
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump pitch_bend(uint8_t channel, uint16_t value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_pitch_bend_direct(0, channel, u16_to_u32(value));
  cmidi2_reverse(msg, u.data);
  return u;
}
}

NAMESPACE_LIBREMIDI::from_01
{
inline libremidi::ump note_on(uint8_t channel, uint8_t pitch, double vel)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_note_on(
      0, channel, pitch, 0, std::clamp(vel, 0., 1.) * std::numeric_limits<uint16_t>::max(), 0);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump note_off(uint8_t channel, uint8_t pitch, double vel)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_note_off(
      0, channel, pitch, 0, std::clamp(vel, 0., 1.) * std::numeric_limits<uint16_t>::max(), 0);
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump poly_pressure(uint8_t channel, uint8_t pitch, double value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_paf(
      0, channel, pitch, std::clamp(value, 0., 1.) * std::numeric_limits<uint32_t>::max());
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump control_change(uint8_t channel, uint8_t index, double value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_cc(
      0, channel, index, std::clamp(value, 0., 1.) * std::numeric_limits<uint32_t>::max());
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump aftertouch(uint8_t channel, double value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_caf(
      0, channel, std::clamp(value, 0., 1.) * std::numeric_limits<uint32_t>::max());
  cmidi2_reverse(msg, u.data);
  return u;
}

inline libremidi::ump pitch_bend(uint8_t channel, double value)
{
  libremidi::ump u;
  const int64_t msg = cmidi2_ump_midi2_pitch_bend_direct(
      0, channel, std::clamp(value, 0., 1.) * std::numeric_limits<uint32_t>::max());
  cmidi2_reverse(msg, u.data);
  return u;
}
}

NAMESPACE_LIBREMIDI::as_midi1
{
inline auto note_off(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.note = cmidi2_ump_get_midi2_note_note(ump);
  res.velocity = cmidi2_ump_get_midi2_note_velocity(ump) / 0x200;
  return res;
}

inline auto note_on(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.note = cmidi2_ump_get_midi2_note_note(ump);
  res.velocity = cmidi2_ump_get_midi2_note_velocity(ump) / 0x200;
  return res;
}

inline auto poly_pressure(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t note;
    uint8_t value;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.note = cmidi2_ump_get_midi2_paf_note(ump);
  res.value = cmidi2_ump_get_midi2_paf_data(ump) / 0x2000000;
  return res;
}

inline auto control_change(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t control;
    uint8_t value;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.control = cmidi2_ump_get_midi2_cc_index(ump);
  res.value = cmidi2_ump_get_midi2_cc_data(ump) / 0x2000000;
  return res;
}

inline auto program_change(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t program;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.program = cmidi2_ump_get_midi2_program_program(ump);
  return res;
}

inline auto aftertouch(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t value;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.value = cmidi2_ump_get_midi2_caf_data(ump) / 0x2000000;
  return res;
}

inline auto pitch_bend(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint16_t value;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.value = cmidi2_ump_get_midi2_pitch_bend_data(ump) / 0x40000;
  return res;
}
}

NAMESPACE_LIBREMIDI::as_01
{
inline auto note_off(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t note;
    double velocity;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.note = cmidi2_ump_get_midi2_note_note(ump);
  res.velocity
      = double(cmidi2_ump_get_midi2_note_velocity(ump)) / std::numeric_limits<uint16_t>::max();
  return res;
}

inline auto note_on(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t note;
    double velocity;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.note = cmidi2_ump_get_midi2_note_note(ump);
  res.velocity
      = double(cmidi2_ump_get_midi2_note_velocity(ump)) / std::numeric_limits<uint16_t>::max();
  return res;
}

inline auto poly_pressure(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t note;
    double value;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.note = cmidi2_ump_get_midi2_paf_note(ump);
  res.value = double(cmidi2_ump_get_midi2_paf_data(ump)) / std::numeric_limits<uint32_t>::max();
  return res;
}

inline auto control_change(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    uint8_t control;
    double value;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.control = cmidi2_ump_get_midi2_cc_index(ump);
  res.value = double(cmidi2_ump_get_midi2_cc_data(ump)) / std::numeric_limits<uint32_t>::max();
  return res;
}

inline auto aftertouch(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    double value;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.value = double(cmidi2_ump_get_midi2_caf_data(ump)) / std::numeric_limits<uint32_t>::max();
  return res;
}

inline auto pitch_bend(const libremidi::ump& mess)
{
  struct
  {
    uint8_t channel;
    double value;
  } res;
  auto& ump = mess.data;
  res.channel = mess.get_channel_0_15();
  res.value
      = double(cmidi2_ump_get_midi2_pitch_bend_data(ump)) / std::numeric_limits<uint32_t>::max();
  return res;
}
}

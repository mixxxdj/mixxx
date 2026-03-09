#pragma once
// clang-format off
#include <libremidi/config.hpp>
// clang-format on

#include <libremidi/cmidi2.hpp>
#include <libremidi/error.hpp>
#include <libremidi/message.hpp>
#include <libremidi/ump.hpp>

#include <cmath>

NAMESPACE_LIBREMIDI
{
struct cmidi2_error_domain : public stdx::error_domain
{
public:
  constexpr cmidi2_error_domain() noexcept
      : error_domain{{0x636d696469325f5fULL, 0x636f6e7665727400ULL}}
  {
  }

  stdx::string_ref name() const noexcept override { return "cmidi2_conversion"; }

  bool equivalent(const stdx::error& lhs, const stdx::error& rhs) const noexcept override
  {
    if (lhs.domain() == rhs.domain())
      return error_cast<cmidi2_midi_conversion_result>(lhs)
             == error_cast<cmidi2_midi_conversion_result>(rhs);

    return false;
  }

  stdx::string_ref message(const stdx::error& e) const noexcept override
  {
    const auto status = error_cast<cmidi2_midi_conversion_result>(e);
    switch (status)
    {
      case CMIDI2_CONVERSION_RESULT_OK:
        return "Success";
      case CMIDI2_CONVERSION_RESULT_OUT_OF_SPACE:
        return "Destination buffer is too small";
      case CMIDI2_CONVERSION_RESULT_INVALID_SYSEX:
        return "Invalid System Exclusive message format";
      case CMIDI2_CONVERSION_RESULT_INVALID_DTE_SEQUENCE:
        return "Invalid Data Entry, RPN, or NRPN sequence";
      case CMIDI2_CONVERSION_RESULT_INVALID_STATUS:
        return "Invalid or unsupported MIDI status byte";
      case CMIDI2_CONVERSION_RESULT_INCOMPLETE_SYSEX7:
        return "Incomplete 7-bit System Exclusive message";
      case CMIDI2_CONVERSION_RESULT_INVALID_INPUT:
        return "Invalid input data length or content";
      default:
        return "Unknown cmidi2 conversion error";
    }
  }
};

inline stdx::error from_cmidi2_result(cmidi2_midi_conversion_result ret) noexcept
{
  static constexpr cmidi2_error_domain domain{};
  // We explicitly map OK to success (0 value error) just in case,
  // though usually error{} is default constructed for success.
  if (ret == CMIDI2_CONVERSION_RESULT_OK)
    return stdx::error{};

  return {ret, domain};
}
}

LIBREMIDI_STATIC void cmidi2_reverse(int64_t v, cmidi2_ump* output)
{
  union
  {
    uint32_t u[2];
    int64_t i;
  } e0, e1;
  e0.i = v;
  e1.u[0] = e0.u[1];
  e1.u[1] = e0.u[0];
  memcpy(output, &e1, 8);
}

LIBREMIDI_STATIC auto u7_to_u16(uint8_t in) -> uint16_t
{
  static constexpr auto ratio = float(std::numeric_limits<uint16_t>::max()) / 127.f;
  return std::clamp(std::round(in * ratio), 0.f, float(std::numeric_limits<uint16_t>::max()));
};

LIBREMIDI_STATIC auto u7_to_u32(uint8_t in) -> uint32_t
{
  static constexpr auto ratio = double(std::numeric_limits<uint32_t>::max()) / 127.;
  return std::clamp(std::round(in * ratio), 0., double(std::numeric_limits<uint32_t>::max()));
};

LIBREMIDI_STATIC auto u16_to_u32(uint16_t in) -> uint32_t
{
  static constexpr auto ratio = double(std::numeric_limits<uint32_t>::max())
                                / double(std::numeric_limits<uint16_t>::max());
  return std::clamp(std::round(in * ratio), 0., double(std::numeric_limits<uint32_t>::max()));
};

LIBREMIDI_STATIC bool
cmidi2_midi1_channel_voice_to_midi2(const uint8_t* bytes, std::size_t sz, cmidi2_ump* output)
{
  if (sz < 2)
    return false;
  if (sz > 3)
    return false;
  const auto status = libremidi::message_type(bytes[0] & 0xF0);
  const int channel = bytes[0] & 0x0F;
  const int group = 0;
  switch (status)
  {
    case libremidi::message_type::NOTE_OFF: {
      const int64_t msg
          = cmidi2_ump_midi2_note_off(group, channel, bytes[1], 0, u7_to_u16(bytes[2]), 0);
      cmidi2_reverse(msg, output);
      break;
    }
    case libremidi::message_type::NOTE_ON: {
      const int64_t msg
          = cmidi2_ump_midi2_note_on(group, channel, bytes[1], 0, u7_to_u16(bytes[2]), 0);
      cmidi2_reverse(msg, output);
      break;
    }
    case libremidi::message_type::POLY_PRESSURE: {
      const int64_t msg = cmidi2_ump_midi2_paf(group, channel, bytes[1], u7_to_u32(bytes[2]));
      cmidi2_reverse(msg, output);
      break;
    }
    case libremidi::message_type::CONTROL_CHANGE: {
      const int64_t msg = cmidi2_ump_midi2_cc(group, channel, bytes[1], u7_to_u32(bytes[2]));
      cmidi2_reverse(msg, output);
      break;
    }
    case libremidi::message_type::PROGRAM_CHANGE: {
      const int64_t msg = cmidi2_ump_midi2_program(group, channel, 0, bytes[1], 0, 0);
      cmidi2_reverse(msg, output);
      break;
    }
    case libremidi::message_type::AFTERTOUCH: {
      const int64_t msg = cmidi2_ump_midi2_caf(group, channel, u7_to_u32(bytes[1]));
      cmidi2_reverse(msg, output);
      break;
    }
    case libremidi::message_type::PITCH_BEND: {
      const auto pb = bytes[1] + bytes[2] * 0x80;
      const int64_t msg = cmidi2_ump_midi2_pitch_bend_direct(group, channel, u16_to_u32(pb));
      cmidi2_reverse(msg, output);
      break;
    }
    default:
      return false;
  }

  return true;
}

LIBREMIDI_STATIC bool
cmidi2_ump_upgrade_midi1_channel_voice_to_midi2(const cmidi2_ump* input, cmidi2_ump* output)
{
  if (cmidi2_ump_get_message_type(input) != CMIDI2_MESSAGE_TYPE_MIDI_1_CHANNEL)
    return false;

  const auto group = cmidi2_ump_get_group(input);
  const auto channel = cmidi2_ump_get_channel(input);
  const auto status = cmidi2_ump_get_status_code(input);
  const auto b1 = cmidi2_ump_get_midi1_byte2(input);
  const auto b2 = cmidi2_ump_get_midi1_byte3(input);

  switch (status)
  {
    // Not in midi 1:
    case CMIDI2_STATUS_PER_NOTE_RCC:
    case CMIDI2_STATUS_PER_NOTE_ACC:
    case CMIDI2_STATUS_RPN:
    case CMIDI2_STATUS_NRPN:
    case CMIDI2_STATUS_RELATIVE_RPN:
    case CMIDI2_STATUS_RELATIVE_NRPN:
    case CMIDI2_STATUS_PER_NOTE_PITCH_BEND:
    case CMIDI2_STATUS_PER_NOTE_MANAGEMENT:
      return false;

    case CMIDI2_STATUS_NOTE_OFF: {
      const int64_t msg = cmidi2_ump_midi2_note_off(group, channel, b1, 0, u7_to_u16(b2), 0);
      cmidi2_reverse(msg, output);
      break;
    }
    case CMIDI2_STATUS_NOTE_ON: {
      const int64_t msg = cmidi2_ump_midi2_note_on(group, channel, b1, 0, u7_to_u16(b2), 0);
      cmidi2_reverse(msg, output);
      break;
    }
    case CMIDI2_STATUS_PAF: {
      const int64_t msg = cmidi2_ump_midi2_paf(group, channel, b1, u7_to_u32(b2));
      cmidi2_reverse(msg, output);
      break;
    }
    case CMIDI2_STATUS_CC: {
      int64_t msg = cmidi2_ump_midi2_cc(group, channel, b1, u7_to_u32(b2));
      cmidi2_reverse(msg, output);
      break;
    }
    case CMIDI2_STATUS_PROGRAM: {
      const int64_t msg = cmidi2_ump_midi2_program(group, channel, 0, b1, 0, 0);
      cmidi2_reverse(msg, output);
      break;
    }
    case CMIDI2_STATUS_CAF: {
      const int64_t msg = cmidi2_ump_midi2_caf(group, channel, u7_to_u32(b1));
      cmidi2_reverse(msg, output);
      break;
    }
    case CMIDI2_STATUS_PITCH_BEND: {
      const auto pb = b1 + b2 * 0x80;
      const int64_t msg = cmidi2_ump_midi2_pitch_bend_direct(group, channel, u16_to_u32(pb));
      cmidi2_reverse(msg, output);
      break;
    }
  }

  return true;
}

NAMESPACE_LIBREMIDI
{
LIBREMIDI_STATIC libremidi::message midi1_from_ump(libremidi::ump u)
{
  libremidi::message ret;
  ret.bytes.resize(4);
  if (auto n = cmidi2_convert_single_ump_to_midi1((uint8_t*)ret.bytes.data(), 4, u.data))
  {
    ret.timestamp = u.timestamp;
    ret.bytes.resize(n);
  }
  else
  {
    ret.bytes.clear();
  }
  return ret;
}

LIBREMIDI_STATIC libremidi::ump ump_from_midi1(libremidi::message u)
{
  libremidi::ump ret;
  if (!u.bytes.empty())
  {
    ret.timestamp = u.timestamp;
    cmidi2_midi1_channel_voice_to_midi2(u.bytes.data(), u.bytes.size(), ret.data);
  }
  return ret;
}

struct midi1_to_midi2
{
  stdx::error
  convert(const unsigned char* message, std::size_t size, int64_t timestamp, auto on_ump)
  {
    context.midi1 = const_cast<unsigned char*>(message);
    context.midi1_num_bytes = size;
    context.midi1_proceeded_bytes = 0;
    context.ump = ump;
    context.ump_num_bytes = sizeof(ump);
    context.ump_proceeded_bytes = 0;
    context.skip_delta_time = true;

    switch (auto err = cmidi2_convert_midi1_to_ump(&context))
    {
      case CMIDI2_CONVERSION_RESULT_OK: {
        // FIXME handle sysex here
        if (auto n = context.ump_proceeded_bytes; n > 0)
          return on_ump(context.ump, context.ump_proceeded_bytes / 4, timestamp);
        else
          return std::errc::no_message;
      }
      case CMIDI2_CONVERSION_RESULT_OUT_OF_SPACE:
      case CMIDI2_CONVERSION_RESULT_INVALID_SYSEX:
      case CMIDI2_CONVERSION_RESULT_INVALID_DTE_SEQUENCE:
      case CMIDI2_CONVERSION_RESULT_INVALID_STATUS:
      case CMIDI2_CONVERSION_RESULT_INCOMPLETE_SYSEX7:
      case CMIDI2_CONVERSION_RESULT_INVALID_INPUT:
        return from_cmidi2_result(err);
      default:
        return std::errc::operation_not_supported;
    }
  }

  cmidi2_midi_conversion_context context = [] {
    cmidi2_midi_conversion_context tmp;
    cmidi2_midi_conversion_context_initialize(&tmp);
    return tmp;
  }();

  uint32_t ump[65536 / 4];
};

struct midi2_to_midi1
{
  stdx::error convert(const uint32_t* message, std::size_t size, int64_t timestamp, auto on_midi)
  {
    context.midi1 = static_cast<unsigned char*>(midi);
    context.midi1_num_bytes = sizeof(midi);
    context.midi1_proceeded_bytes = 0;
    context.ump = const_cast<uint32_t*>(message);
    context.ump_num_bytes = size * sizeof(uint32_t);
    context.ump_proceeded_bytes = 0;
    context.skip_delta_time = true;

    switch (auto err = cmidi2_convert_ump_to_midi1(&context))
    {
      case CMIDI2_CONVERSION_RESULT_OK: {
        if (auto n = context.midi1_proceeded_bytes; n > 0)
          return on_midi(midi, n, timestamp);
        else
          return std::errc::no_message;
      }
      case CMIDI2_CONVERSION_RESULT_OUT_OF_SPACE:
      case CMIDI2_CONVERSION_RESULT_INVALID_SYSEX:
      case CMIDI2_CONVERSION_RESULT_INVALID_DTE_SEQUENCE:
      case CMIDI2_CONVERSION_RESULT_INVALID_STATUS:
      case CMIDI2_CONVERSION_RESULT_INCOMPLETE_SYSEX7:
      case CMIDI2_CONVERSION_RESULT_INVALID_INPUT:
        return from_cmidi2_result(err);
      default:
        return std::errc::operation_not_supported;
    }
  }

  cmidi2_midi_conversion_context context = [] {
    cmidi2_midi_conversion_context tmp;
    cmidi2_midi_conversion_context_initialize(&tmp);
    return tmp;
  }();

  uint8_t midi[65536];
};

}

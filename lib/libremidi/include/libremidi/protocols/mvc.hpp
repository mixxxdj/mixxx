#pragma once

#include <libremidi/detail/polyfill.hpp>
#include <libremidi/error_handler.hpp>
#include <libremidi/message.hpp>

#include <cstdint>
#include <cstdio>
#include <functional>
#include <span>

// MIDI Visual Control (MVC)
// MMA Recommended Practice RP-050, CA-032
NAMESPACE_LIBREMIDI
{
struct mvc_protocol
{
  // Default CC assignments from RP-050 Section 2.2.1
  // These are reassignable via SysEx; these are the factory defaults.
  // Effect Control meaning depends on color space:
  //   RGB:   Effect 1=Red, 2=Blue, 3=Green
  //   HSB:   Effect 1=Saturation, 2=Hue, 3=Brightness
  //   YCbCr: Effect 1=Cr, 2=Cb, 3=Y
  enum class control : uint8_t
  {
    bank_select_msb = 0x00,   // CC 0 - Image bank select MSB
    dissolve_time_msb = 0x05, // CC 5 - Dissolve/transition time MSB
    bank_select_lsb = 0x20,   // CC 32 - Image bank select LSB
    dissolve_time_lsb = 0x25, // CC 37 - Dissolve/transition time LSB
    effect_control_1 = 0x47,  // CC 71
    effect_control_2 = 0x49,  // CC 73
    effect_control_3 = 0x4A,  // CC 74
    reset = 0x79,             // CC 121 - Reset All Controllers to defaults
  };

  // MVC Data Set parameter addresses (RP-050 Section 2.3)
  static constexpr uint8_t addr_mvc_on_off[3] = {0x10, 0x00, 0x00};

  // Parsed event types for variant-based dispatch
  struct enable_mvc
  {
    uint8_t target_device;
  };
  struct disable_mvc
  {
    uint8_t target_device;
  };
  struct clip_trigger
  {
    uint8_t channel;
    uint8_t note;
    bool pressed;
  };
  struct control_change
  {
    uint8_t channel;
    uint8_t cc;
    uint8_t value;
  };
  struct clip_select
  {
    uint8_t channel;
    uint8_t program;
  };
  struct playback_speed
  {
    uint8_t channel;
    uint16_t value; // 14-bit, center (0x2000) = normal speed
  };

  using event_variant = libremidi::variant<
      enable_mvc, disable_mvc, clip_trigger, control_change, clip_select, playback_speed>;

  uint8_t device_id = 0x00; // MVC default device ID

  static libremidi::message make_command_impl(auto&&... data)
  {
    using namespace std;
    libremidi::message m;
    m.bytes.reserve((std::ssize(data) + ...));
    (m.bytes.insert(m.bytes.end(), begin(data), end(data)), ...);
    return m;
  }

  // Compute checksum for MVC Data Set (Roland-style):
  // checksum = (128 - (sum_of_address_and_data % 128)) % 128
  static uint8_t compute_checksum(std::span<const uint8_t> data)
  {
    uint8_t sum = 0;
    for (auto b : data)
      sum += b;
    return (128 - (sum % 128)) % 128;
  }

  // Build MVC Data Set SysEx (RP-050 Section 2.3.2):
  // F0 7E <dev> 0C 01 <addr_hi> <addr_mid> <addr_lo> <data...> <checksum> F7
  libremidi::message
  make_data_set(uint8_t addr_hi, uint8_t addr_mid, uint8_t addr_lo, uint8_t value)
  {
    const uint8_t payload[4]{addr_hi, addr_mid, addr_lo, value};
    const uint8_t cksum = compute_checksum(payload);
    const uint8_t msg[11]{
        0xF0, 0x7E, device_id, 0x0C, 0x01, addr_hi, addr_mid, addr_lo, value, cksum, 0xF7};
    return make_command_impl(msg);
  }

  // MVC ON: address 10 00 00, data 01
  libremidi::message make_enable() { return make_data_set(0x10, 0x00, 0x00, 0x01); }

  // MVC OFF: address 10 00 00, data 00
  libremidi::message make_disable() { return make_data_set(0x10, 0x00, 0x00, 0x00); }
};

struct mvc_configuration
{
  //! How to send MIDI messages to the device.
  //! Note: this function *will* be called from different threads,
  //! thus it has to be thread-safe.
  std::function<void(libremidi::message&&)> midi_out;

  //! Called when an MVC event is received.
  std::function<void(const mvc_protocol::event_variant&)> on_event;

  libremidi::midi_error_callback on_error{};
};

struct mvc_processor : libremidi::error_handler
{
  mvc_configuration configuration;
  mvc_protocol impl;

  explicit mvc_processor(mvc_configuration conf)
      : configuration{std::move(conf)}
  {
    assert(configuration.midi_out);

    if (!configuration.on_error)
      configuration.on_error = [](std::string_view s, auto&&...) {
        std::fprintf(stderr, "libremidi: mvc error: %s\n", s.data());
      };

    if (!configuration.on_event)
      configuration.on_event = [](auto&&...) {};
  }

  // Output
  void enable() { configuration.midi_out(impl.make_enable()); }
  void disable() { configuration.midi_out(impl.make_disable()); }

  void trigger_clip(uint8_t channel, uint8_t note, uint8_t velocity = 127)
  {
    using ce = libremidi::channel_events;
    configuration.midi_out(ce::note_on(channel, note, velocity));
  }

  void stop_clip(uint8_t channel, uint8_t note)
  {
    using ce = libremidi::channel_events;
    configuration.midi_out(ce::note_off(channel, note, 0));
  }

  void set_control(uint8_t channel, uint8_t cc, uint8_t value)
  {
    using ce = libremidi::channel_events;
    configuration.midi_out(ce::control_change(channel, cc, value));
  }

  void select_clip(uint8_t channel, uint8_t program)
  {
    using ce = libremidi::channel_events;
    configuration.midi_out(ce::program_change(channel, program));
  }

  void set_playback_speed(uint8_t channel, uint16_t value)
  {
    using ce = libremidi::channel_events;
    configuration.midi_out(ce::pitch_bend(channel, static_cast<int>(value)));
  }

  // Input
  void on_midi(const libremidi::message& message)
  {
    auto type = message.get_message_type();

    // SysEx Data Set (RP-050 Section 2.3):
    //   with checksum: F0 7E <dev> 0C 01 <addr * 3> <data...> <cksum> F7  (>= 11 bytes)
    //   without:       F0 7E <dev> 0C 01 <addr * 3> <data...> F7          (>= 10 bytes)
    // Some implementations (e.g. tschiemer/midimessage) omit the checksum.
    if (type == libremidi::message_type::SYSTEM_EXCLUSIVE && message.size() >= 10)
    {
      const uint8_t* bytes = message.bytes.data();
      if (bytes[1] == 0x7E && (bytes[2] == impl.device_id || bytes[2] == 0x7F)
          && bytes[3] == 0x0C && bytes[4] == 0x01)
      {
        // Verify checksum if present (message has room for addr + data + checksum)
        if (message.size() >= 11)
        {
          const size_t payload_len
              = message.size() - 7; // exclude F0 7E dev 0C 01 ... cksum F7
          const uint8_t expected = mvc_protocol::compute_checksum(
              std::span<const uint8_t>(bytes + 5, payload_len));
          const uint8_t actual = bytes[message.size() - 2];
          if (expected != actual)
            return;
        }

        // Parse MVC On/Off: address 10 00 00
        if (bytes[5] == 0x10 && bytes[6] == 0x00 && bytes[7] == 0x00)
        {
          if (bytes[8] == 0x01)
            configuration.on_event(mvc_protocol::enable_mvc{bytes[2]});
          else if (bytes[8] == 0x00)
            configuration.on_event(mvc_protocol::disable_mvc{bytes[2]});
        }
      }
      return;
    }

    // Channel voice messages
    uint8_t ch = message.get_channel();
    switch (type)
    {
      case libremidi::message_type::NOTE_ON:
        configuration.on_event(mvc_protocol::clip_trigger{ch, message[1], message[2] > 0});
        break;
      case libremidi::message_type::NOTE_OFF:
        configuration.on_event(mvc_protocol::clip_trigger{ch, message[1], false});
        break;
      case libremidi::message_type::CONTROL_CHANGE:
        configuration.on_event(mvc_protocol::control_change{ch, message[1], message[2]});
        break;
      case libremidi::message_type::PROGRAM_CHANGE:
        configuration.on_event(mvc_protocol::clip_select{ch, message[1]});
        break;
      case libremidi::message_type::PITCH_BEND: {
        uint16_t value = message.bytes[2] * 128 + message.bytes[1];
        configuration.on_event(mvc_protocol::playback_speed{ch, value});
        break;
      }
      default:
        break;
    }
  }
};
}

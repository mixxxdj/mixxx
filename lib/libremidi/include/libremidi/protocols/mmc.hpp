#pragma once

#include <libremidi/detail/polyfill.hpp>
#include <libremidi/error_handler.hpp>
#include <libremidi/message.hpp>

#include <cstdint>
#include <cstdio>
#include <functional>
#include <span>

// MIDI Machine Control (MMC)
// MMA Recommended Practice RP-013
NAMESPACE_LIBREMIDI
{
struct mmc_protocol
{
  // Wire protocol command byte identifiers
  enum class command_id : uint8_t
  {
    stop = 0x01,
    play = 0x02,
    deferred_play = 0x03,
    fast_forward = 0x04,
    rewind = 0x05,
    record_strobe = 0x06,
    record_exit = 0x07,
    record_pause = 0x08,
    pause = 0x09,
    eject = 0x0A,
    chase = 0x0B,
    command_error_reset = 0x0C,
    mmc_reset = 0x0D,
    locate = 0x44,
  };

  // Parsed command types for variant-based dispatch
  struct stop
  {
    uint8_t target_device;
  };
  struct play
  {
    uint8_t target_device;
  };
  struct deferred_play
  {
    uint8_t target_device;
  };
  struct fast_forward
  {
    uint8_t target_device;
  };
  struct rewind
  {
    uint8_t target_device;
  };
  struct record_strobe
  {
    uint8_t target_device;
  };
  struct record_exit
  {
    uint8_t target_device;
  };
  struct record_pause
  {
    uint8_t target_device;
  };
  struct pause
  {
    uint8_t target_device;
  };
  struct eject
  {
    uint8_t target_device;
  };
  struct chase
  {
    uint8_t target_device;
  };
  struct command_error_reset
  {
    uint8_t target_device;
  };
  struct mmc_reset
  {
    uint8_t target_device;
  };
  struct locate
  {
    uint8_t target_device;
    // hr bits 5-6 encode SMPTE type: 00=24fps, 01=25fps, 10=30fps drop, 11=30fps
    uint8_t hr, min, sec, frames, subframes;
  };

  using command_variant = libremidi::variant<
      stop, play, deferred_play, fast_forward, rewind, record_strobe, record_exit, record_pause,
      pause, eject, chase, command_error_reset, mmc_reset, locate>;

  uint8_t device_id = 0x7F; // 0x7F = broadcast "All Call"

  static libremidi::message make_command_impl(auto&&... data)
  {
    using namespace std;
    libremidi::message m;
    m.bytes.reserve((std::ssize(data) + ...));
    (m.bytes.insert(m.bytes.end(), begin(data), end(data)), ...);
    return m;
  }

  // F0 7F <device_id> 06 <command> [<payload>] F7
  libremidi::message make_command(command_id c, std::span<const uint8_t> payload = {})
  {
    const uint8_t header[5]{0xF0, 0x7F, device_id, 0x06, to_underlying(c)};
    const uint8_t footer[1]{0xF7};
    return make_command_impl(header, payload, footer);
  }

  // Locate: F0 7F <device_id> 06 44 06 01 <hr> <min> <sec> <frames> <subframes> F7
  libremidi::message
  make_locate(uint8_t hr, uint8_t min, uint8_t sec, uint8_t frames, uint8_t subframes)
  {
    const uint8_t payload[7]{0x06, 0x01, hr, min, sec, frames, subframes};
    return make_command(command_id::locate, payload);
  }
};

struct mmc_configuration
{
  //! How to send MIDI messages to the device.
  //! Note: this function *will* be called from different threads,
  //! thus it has to be thread-safe.
  std::function<void(libremidi::message&&)> midi_out;

  //! Called when an MMC command is received.
  std::function<void(const mmc_protocol::command_variant&)> on_command;

  libremidi::midi_error_callback on_error{};
};

struct mmc_processor : libremidi::error_handler
{
  mmc_configuration configuration;
  mmc_protocol impl;

  explicit mmc_processor(mmc_configuration conf)
      : configuration{std::move(conf)}
  {
    assert(configuration.midi_out);

    if (!configuration.on_error)
      configuration.on_error = [](std::string_view s, auto&&...) {
        std::fprintf(stderr, "libremidi: mmc error: %s\n", s.data());
      };

    if (!configuration.on_command)
      configuration.on_command = [](auto&&...) {};
  }

  // Output
  void send(mmc_protocol::command_id c) { configuration.midi_out(impl.make_command(c)); }

  void locate(uint8_t hr, uint8_t min, uint8_t sec, uint8_t frames, uint8_t subframes)
  {
    configuration.midi_out(impl.make_locate(hr, min, sec, frames, subframes));
  }

  // Input
  void on_midi(const libremidi::message& message)
  {
    if (message.get_message_type() != libremidi::message_type::SYSTEM_EXCLUSIVE)
      return;
    if (message.size() < 6)
      return;

    const uint8_t* bytes = message.bytes.data();

    // F0 7F <id> 06 <cmd> ... F7
    if (bytes[1] != 0x7F)
      return;
    if (bytes[2] != impl.device_id && bytes[2] != 0x7F)
      return;
    if (bytes[3] != 0x06)
      return;

    const uint8_t target = bytes[2];

    switch (bytes[4])
    {
      case 0x01:
        configuration.on_command(mmc_protocol::stop{target});
        break;
      case 0x02:
        configuration.on_command(mmc_protocol::play{target});
        break;
      case 0x03:
        configuration.on_command(mmc_protocol::deferred_play{target});
        break;
      case 0x04:
        configuration.on_command(mmc_protocol::fast_forward{target});
        break;
      case 0x05:
        configuration.on_command(mmc_protocol::rewind{target});
        break;
      case 0x06:
        configuration.on_command(mmc_protocol::record_strobe{target});
        break;
      case 0x07:
        configuration.on_command(mmc_protocol::record_exit{target});
        break;
      case 0x08:
        configuration.on_command(mmc_protocol::record_pause{target});
        break;
      case 0x09:
        configuration.on_command(mmc_protocol::pause{target});
        break;
      case 0x0A:
        configuration.on_command(mmc_protocol::eject{target});
        break;
      case 0x0B:
        configuration.on_command(mmc_protocol::chase{target});
        break;
      case 0x0C:
        configuration.on_command(mmc_protocol::command_error_reset{target});
        break;
      case 0x0D:
        configuration.on_command(mmc_protocol::mmc_reset{target});
        break;
      case 0x44: // Locate: 44 06 01 <hr> <min> <sec> <frames> <subframes>
        if (message.size() >= 13 && bytes[5] == 0x06 && bytes[6] == 0x01)
        {
          configuration.on_command(
              mmc_protocol::locate{target, bytes[7], bytes[8], bytes[9], bytes[10], bytes[11]});
        }
        break;
      default:
        break;
    }
  }
};
}

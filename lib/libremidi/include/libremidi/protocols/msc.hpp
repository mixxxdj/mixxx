#pragma once

#include <libremidi/detail/polyfill.hpp>
#include <libremidi/error_handler.hpp>
#include <libremidi/message.hpp>

#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <string_view>

// MIDI Show Control (MSC)
// MMA Recommended Practice RP-002/014
NAMESPACE_LIBREMIDI
{
struct msc_protocol
{
  // Equipment type addressed by the command (RP-002 Table)
  enum class command_format : uint8_t
  {
    // Lighting (0x01-0x06)
    lighting = 0x01,
    moving_lights = 0x02,
    color_changers = 0x03,
    strobes = 0x04,
    lasers = 0x05,
    chasers = 0x06,

    // Sound (0x10-0x18)
    sound = 0x10,
    music = 0x11,
    cd_players = 0x12,
    eprom_playback = 0x13,
    audio_tape_machines = 0x14,
    intercoms = 0x15,
    amplifiers = 0x16,
    audio_effects = 0x17,
    equalisers = 0x18,

    // Machinery (0x20-0x2A)
    machinery = 0x20,
    rigging = 0x21,
    flys = 0x22,
    lifts = 0x23,
    turntables = 0x24,
    trusses = 0x25,
    robots = 0x26,
    animation = 0x27,
    floats = 0x28,
    breakaways = 0x29,
    barges = 0x2A,

    // Video (0x30)
    video = 0x30,

    // Projection (0x40-0x45)
    projection = 0x40,
    film_projectors = 0x41,
    slide_projectors = 0x42,
    video_projectors = 0x43,
    dissolvers = 0x44,
    shutter_controls = 0x45,

    // Process Control (0x50-0x58)
    process_control = 0x50,
    hydraulic_oil = 0x51,
    h2o = 0x52,
    co2 = 0x53,
    compressed_air = 0x54,
    natural_gas = 0x55,
    fog = 0x56,
    smoke = 0x57,
    cracked_haze = 0x58,

    // Pyrotechnics (0x60-0x64)
    pyro = 0x60,
    fireworks = 0x61,
    explosions = 0x62,
    flame = 0x63,
    smoke_pots = 0x64,

    all_types = 0x7F,
  };

  // Wire protocol command byte identifiers
  enum class command_id : uint8_t
  {
    // General commands (0x01-0x0B)
    go = 0x01,
    stop = 0x02,
    resume = 0x03,
    timed_go = 0x04,
    load = 0x05,
    set = 0x06,
    fire = 0x07,
    all_off = 0x08,
    restore = 0x09,
    reset = 0x0A,
    go_off = 0x0B,

    // Sound commands (0x10-0x1E)
    go_jam_clock = 0x10,
    standby_plus = 0x11,
    standby_minus = 0x12,
    sequence_plus = 0x13,
    sequence_minus = 0x14,
    start_clock = 0x15,
    stop_clock = 0x16,
    zero_clock = 0x17,
    set_clock = 0x18,
    mtc_chase_on = 0x19,
    mtc_chase_off = 0x1A,
    open_cue_list = 0x1B,
    close_cue_list = 0x1C,
    open_cue_path = 0x1D,
    close_cue_path = 0x1E,
  };

  // Cue information common to most MSC commands.
  // Cue numbers/lists/paths are ASCII strings on the wire, separated by 0x00.
  struct cue_data
  {
    uint8_t target_device{};
    command_format format{};
    std::string cue_number;
    std::string cue_list;
    std::string cue_path;
  };

  // Parsed command types for variant-based dispatch
  // General commands (0x01-0x0B)
  struct go : cue_data
  {
  };
  struct stop : cue_data
  {
  };
  struct resume : cue_data
  {
  };
  struct load : cue_data
  {
  };
  struct set : cue_data
  {
  };
  struct fire : cue_data
  {
  };
  struct all_off : cue_data
  {
  };
  struct restore : cue_data
  {
  };
  struct reset : cue_data
  {
  };
  struct go_off : cue_data
  {
  };

  // Timed Go includes SMPTE time before the cue data
  struct timed_go : cue_data
  {
    uint8_t hours{};
    uint8_t minutes{};
    uint8_t seconds{};
    uint8_t frames{};
    uint8_t fractional_frames{};
  };

  // Sound commands (0x10-0x1E)
  struct go_jam_clock : cue_data
  {
  };
  struct standby_plus : cue_data
  {
  };
  struct standby_minus : cue_data
  {
  };
  struct sequence_plus : cue_data
  {
  };
  struct sequence_minus : cue_data
  {
  };
  struct start_clock : cue_data
  {
  };
  struct stop_clock : cue_data
  {
  };
  struct zero_clock : cue_data
  {
  };
  struct set_clock : cue_data
  {
  };
  struct mtc_chase_on : cue_data
  {
  };
  struct mtc_chase_off : cue_data
  {
  };
  struct open_cue_list : cue_data
  {
  };
  struct close_cue_list : cue_data
  {
  };
  struct open_cue_path : cue_data
  {
  };
  struct close_cue_path : cue_data
  {
  };

  using command_variant = libremidi::variant<
      go, stop, resume, timed_go, load, set, fire, all_off, restore, reset, go_off,
      go_jam_clock, standby_plus, standby_minus, sequence_plus, sequence_minus, start_clock,
      stop_clock, zero_clock, set_clock, mtc_chase_on, mtc_chase_off, open_cue_list,
      close_cue_list, open_cue_path, close_cue_path>;

  uint8_t device_id = 0x7F;
  command_format format = command_format::all_types;

  // F0 7F <device_id> 02 <format> <command> [Q_number [00 Q_list [00 Q_path]]] F7
  libremidi::message make_command(
      command_id c, std::string_view cue_number = {}, std::string_view cue_list = {},
      std::string_view cue_path = {})
  {
    libremidi::message m;
    m.bytes = {0xF0, 0x7F, device_id, 0x02, to_underlying(format), to_underlying(c)};

    for (char ch : cue_number)
      m.bytes.push_back(static_cast<uint8_t>(ch));

    if (!cue_list.empty())
    {
      m.bytes.push_back(0x00);
      for (char ch : cue_list)
        m.bytes.push_back(static_cast<uint8_t>(ch));
    }
    if (!cue_path.empty())
    {
      m.bytes.push_back(0x00);
      for (char ch : cue_path)
        m.bytes.push_back(static_cast<uint8_t>(ch));
    }

    m.bytes.push_back(0xF7);
    return m;
  }

  // F0 7F <device_id> 02 <format> 04 <hh> <mm> <ss> <fr> <ff> [cue_data] F7
  libremidi::message make_timed_go(
      uint8_t hh, uint8_t mm, uint8_t ss, uint8_t fr, uint8_t ff,
      std::string_view cue_number = {}, std::string_view cue_list = {},
      std::string_view cue_path = {})
  {
    libremidi::message m;
    m.bytes = {0xF0, 0x7F, device_id, 0x02, to_underlying(format),
               to_underlying(command_id::timed_go), hh, mm, ss, fr, ff};

    for (char ch : cue_number)
      m.bytes.push_back(static_cast<uint8_t>(ch));

    if (!cue_list.empty())
    {
      m.bytes.push_back(0x00);
      for (char ch : cue_list)
        m.bytes.push_back(static_cast<uint8_t>(ch));
    }
    if (!cue_path.empty())
    {
      m.bytes.push_back(0x00);
      for (char ch : cue_path)
        m.bytes.push_back(static_cast<uint8_t>(ch));
    }

    m.bytes.push_back(0xF7);
    return m;
  }
};

struct msc_configuration
{
  //! How to send MIDI messages to the device.
  //! Note: this function *will* be called from different threads,
  //! thus it has to be thread-safe.
  std::function<void(libremidi::message&&)> midi_out;

  //! Called when an MSC command is received.
  std::function<void(const msc_protocol::command_variant&)> on_command;

  libremidi::midi_error_callback on_error{};
};

struct msc_processor : libremidi::error_handler
{
  msc_configuration configuration;
  msc_protocol impl;

  explicit msc_processor(msc_configuration conf)
      : configuration{std::move(conf)}
  {
    assert(configuration.midi_out);

    if (!configuration.on_error)
      configuration.on_error = [](std::string_view s, auto&&...) {
        std::fprintf(stderr, "libremidi: msc error: %s\n", s.data());
      };

    if (!configuration.on_command)
      configuration.on_command = [](auto&&...) {};
  }

  // Output
  void send(
      msc_protocol::command_id c, std::string_view cue_number = {},
      std::string_view cue_list = {}, std::string_view cue_path = {})
  {
    configuration.midi_out(impl.make_command(c, cue_number, cue_list, cue_path));
  }

  void timed_go(
      uint8_t hh, uint8_t mm, uint8_t ss, uint8_t fr, uint8_t ff,
      std::string_view cue_number = {}, std::string_view cue_list = {},
      std::string_view cue_path = {})
  {
    configuration.midi_out(
        impl.make_timed_go(hh, mm, ss, fr, ff, cue_number, cue_list, cue_path));
  }

  // Input
  void on_midi(const libremidi::message& message)
  {
    if (message.get_message_type() != libremidi::message_type::SYSTEM_EXCLUSIVE)
      return;
    if (message.size() < 7)
      return;

    const uint8_t* bytes = message.bytes.data();

    // F0 7F <id> 02 <format> <cmd> [<data>] F7
    if (bytes[1] != 0x7F)
      return;
    if (bytes[2] != impl.device_id && bytes[2] != 0x7F)
      return;
    if (bytes[3] != 0x02)
      return;

    const uint8_t target = bytes[2];
    const auto fmt = static_cast<msc_protocol::command_format>(bytes[4]);

    // Parse ASCII cue fields separated by 0x00 starting at the given byte offset
    auto parse_cue = [&](size_t offset) -> msc_protocol::cue_data {
      msc_protocol::cue_data data;
      data.target_device = target;
      data.format = fmt;

      int field = 0;
      std::string* fields[3] = {&data.cue_number, &data.cue_list, &data.cue_path};
      for (size_t i = offset; i < message.size() - 1; ++i)
      {
        if (bytes[i] == 0x00)
        {
          if (++field >= 3)
            break;
        }
        else
        {
          *fields[field] += static_cast<char>(bytes[i]);
        }
      }
      return data;
    };

    switch (bytes[5])
    {
      // General commands (0x01-0x0B)
      case 0x01:
        configuration.on_command(msc_protocol::go{parse_cue(6)});
        break;
      case 0x02:
        configuration.on_command(msc_protocol::stop{parse_cue(6)});
        break;
      case 0x03:
        configuration.on_command(msc_protocol::resume{parse_cue(6)});
        break;
      case 0x04: // timed_go: 5 time bytes before cue data
        if (message.size() >= 12)
        {
          auto data = parse_cue(11);
          configuration.on_command(msc_protocol::timed_go{
              std::move(data), bytes[6], bytes[7], bytes[8], bytes[9], bytes[10]});
        }
        break;
      case 0x05:
        configuration.on_command(msc_protocol::load{parse_cue(6)});
        break;
      case 0x06:
        configuration.on_command(msc_protocol::set{parse_cue(6)});
        break;
      case 0x07:
        configuration.on_command(msc_protocol::fire{parse_cue(6)});
        break;
      case 0x08:
        configuration.on_command(msc_protocol::all_off{parse_cue(6)});
        break;
      case 0x09:
        configuration.on_command(msc_protocol::restore{parse_cue(6)});
        break;
      case 0x0A:
        configuration.on_command(msc_protocol::reset{parse_cue(6)});
        break;
      case 0x0B:
        configuration.on_command(msc_protocol::go_off{parse_cue(6)});
        break;

      // Sound commands (0x10-0x1E)
      case 0x10:
        configuration.on_command(msc_protocol::go_jam_clock{parse_cue(6)});
        break;
      case 0x11:
        configuration.on_command(msc_protocol::standby_plus{parse_cue(6)});
        break;
      case 0x12:
        configuration.on_command(msc_protocol::standby_minus{parse_cue(6)});
        break;
      case 0x13:
        configuration.on_command(msc_protocol::sequence_plus{parse_cue(6)});
        break;
      case 0x14:
        configuration.on_command(msc_protocol::sequence_minus{parse_cue(6)});
        break;
      case 0x15:
        configuration.on_command(msc_protocol::start_clock{parse_cue(6)});
        break;
      case 0x16:
        configuration.on_command(msc_protocol::stop_clock{parse_cue(6)});
        break;
      case 0x17:
        configuration.on_command(msc_protocol::zero_clock{parse_cue(6)});
        break;
      case 0x18:
        configuration.on_command(msc_protocol::set_clock{parse_cue(6)});
        break;
      case 0x19:
        configuration.on_command(msc_protocol::mtc_chase_on{parse_cue(6)});
        break;
      case 0x1A:
        configuration.on_command(msc_protocol::mtc_chase_off{parse_cue(6)});
        break;
      case 0x1B:
        configuration.on_command(msc_protocol::open_cue_list{parse_cue(6)});
        break;
      case 0x1C:
        configuration.on_command(msc_protocol::close_cue_list{parse_cue(6)});
        break;
      case 0x1D:
        configuration.on_command(msc_protocol::open_cue_path{parse_cue(6)});
        break;
      case 0x1E:
        configuration.on_command(msc_protocol::close_cue_path{parse_cue(6)});
        break;
      default:
        break;
    }
  }
};
}

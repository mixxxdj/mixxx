#pragma once

#if !defined(LIBREMIDI_USE_MODULES)
#include <libremidi/libremidi.hpp>
// Credits to https://raw.githubusercontent.com/atsushieno/cmidi2
#include <libremidi/cmidi2.hpp>
#endif
#if LIBREMIDI_USE_NI_MIDI2
  #include <midi/universal_packet.h>
#endif

#include "3rdparty/args.hxx"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <variant>

inline std::ostream& operator<<(std::ostream& s, libremidi::port_information::port_type t)
{
  if (t & libremidi::transport_type::software)
  {
    s << "software";
    if (t & libremidi::transport_type::loopback)
      s << ", loopback";
  }

  if (t & libremidi::transport_type::hardware)
  {
    s << "hardware";
    if (t & libremidi::transport_type::usb)
      s << ", usb";
    if (t & libremidi::transport_type::bluetooth)
      s << ", bt";
    if (t & libremidi::transport_type::pci)
      s << ", pci";
  }

  if (t & libremidi::transport_type::network)
    s << "network";
  return s;
}

inline std::ostream& operator<<(std::ostream& s, const libremidi::container_identifier& id)
{
  struct
  {
    std::ostream& s;
    void operator()(libremidi::uuid u) { s << "uuid"; }
    void operator()(std::string u) { s << u; }
    void operator()(uint64_t u) { s << u; }
    void operator()(libremidi::monostate) { }
  } vis{s};
  visit(vis, id);
  return s;
}

inline std::ostream& operator<<(std::ostream& s, const libremidi::device_identifier& id)
{
  struct
  {
    std::ostream& s;
    void operator()(const std::string& u) { s << u; }
    void operator()(libremidi::usb_device_identifier u)
    {
      s << u.vendor_id << ":" << u.product_id;
    }
    void operator()(uint64_t u)
    {
      auto res = static_cast<uint32_t>(u);
      std::ios_base::fmtflags f(s.flags());
      s << std::hex << std::setfill('0') << std::setw(4) << (res >> 16) << ":" << std::hex
        << std::setfill('0') << std::setw(4) << (res & 0x0000FFFF);
      s.flags(f);
    }
    void operator()(libremidi::monostate) { }
  } vis{s};
  visit(vis, id);
  return s;
}

inline std::ostream& operator<<(std::ostream& s, const libremidi::message& message)
{
  auto nBytes = message.size();
  s << "[ ";
  for (auto i = 0U; i < nBytes; i++)
    s << std::hex << (int)message[i] << std::dec << " ";
  s << "]";
  if (nBytes > 0)
    s << " ; stamp = " << message.timestamp;
  return s;
}

inline std::ostream& operator<<(std::ostream& s, const libremidi::ump& message)
{
  const cmidi2_ump* b = message;
  int bytes = cmidi2_ump_get_num_bytes(message.data[0]);
  int mode = cmidi2_ump_get_message_type(b);
  int group = cmidi2_ump_get_group(b);
  int status = cmidi2_ump_get_status_code(b);
  int channel = cmidi2_ump_get_channel(b);
  auto mode_to_str = [mode]() {
    switch (mode)
    {
      case CMIDI2_MESSAGE_TYPE_UTILITY:
        return "0=utility";
      case CMIDI2_MESSAGE_TYPE_SYSTEM:
        return "1=system";
      case CMIDI2_MESSAGE_TYPE_MIDI_1_CHANNEL:
        return "2=m1channel";
      case CMIDI2_MESSAGE_TYPE_SYSEX7:
        return "3=sysex7";
      case CMIDI2_MESSAGE_TYPE_MIDI_2_CHANNEL:
        return "4=m2channel";
      case CMIDI2_MESSAGE_TYPE_SYSEX8_MDS:
        return "5=sysex8";
      case CMIDI2_MESSAGE_TYPE_FLEX_DATA:
        return "D=flexdata";
      case CMIDI2_MESSAGE_TYPE_UMP_STREAM:
        return "F=stream";
      default:
        return "unknown";
    }
  };
  s << "[ b:" << bytes << " | m:" << mode_to_str() << " | g:" << group;

  switch ((libremidi::message_type)status)
  {
    case libremidi::message_type::NOTE_ON:
      s << " | note on: c" << channel << " n" << (int)cmidi2_ump_get_midi2_note_note(b) << " v"
        << cmidi2_ump_get_midi2_note_velocity(b);
      break;
    case libremidi::message_type::NOTE_OFF:
      s << " | note off: c" << channel << " n" << (int)cmidi2_ump_get_midi2_note_note(b) << " v"
        << cmidi2_ump_get_midi2_note_velocity(b);
      break;
    case libremidi::message_type::CONTROL_CHANGE:
      s << " | cc: c" << channel << " i" << (int)cmidi2_ump_get_midi2_cc_index(b) << " v"
        << cmidi2_ump_get_midi2_cc_data(b);
      break;
    case libremidi::message_type::PITCH_BEND:
      s << " | pb: c" << channel << " v" << (int)cmidi2_ump_get_midi2_pitch_bend_data(b);
      break;
    case libremidi::message_type::POLY_PRESSURE:
      s << " | pp: c" << channel << " n" << (int)cmidi2_ump_get_midi2_paf_note(b) << " v"
        << (int)cmidi2_ump_get_midi2_paf_data(b);
      break;
    case libremidi::message_type::AFTERTOUCH:
      s << " | at: c" << channel << " v" << (int)cmidi2_ump_get_midi2_caf_data(b);
      break;
    case libremidi::message_type::PROGRAM_CHANGE:
      s << " | pc: c" << channel << " v" << (int)cmidi2_ump_get_midi2_program_program(b);
      break;

    default:
      break;
  }
  s << " ]";
#if LIBREMIDI_USE_NI_MIDI2
  s << " :: " << midi::universal_packet(message);
#endif
  return s;
}

inline std::ostream& operator<<(std::ostream& s, const libremidi::port_information& rhs)
{
  s << "[ client: " << rhs.client;
  if (rhs.container.index() > 0)
    s << "\n    , container: " << rhs.container;
  if (rhs.device.index() > 0)
    s << "\n    , device_id: " << rhs.device;

  s << "\n    , port: " << rhs.port;

  if (!rhs.manufacturer.empty())
    s << "\n    , manufacturer: " << rhs.manufacturer;
  if (!rhs.product.empty())
    s << "\n    , product: " << rhs.product;
  if (!rhs.serial.empty())
    s << "\n    , serial: " << rhs.serial;
  if (!rhs.device_name.empty())
    s << "\n    , device_name: " << rhs.device_name;
  if (!rhs.port_name.empty())
    s << "\n    , port_name: " << rhs.port_name;
  if (!rhs.display_name.empty())
    s << "\n    , display_name: " << rhs.display_name;
  if (rhs.type != libremidi::transport_type::unknown)
    s << "\n    , type: " << rhs.type;
  return s << "\n  ]";
}

namespace libremidi::examples
{
struct arguments
{
  libremidi::API api{libremidi::API::UNSPECIFIED};
  int input_port{0};
  int output_port{0};
  int count{50};
  bool virtual_port{};

  static std::string api_list()
  {
    std::string ret;
    ret.reserve(64);
    auto apis = libremidi::available_apis();
    for (auto api : apis)
    {
      ret += libremidi::get_api_name(api);
      ret += ", ";
    }
    if (apis.size() > 0)
      ret.resize(ret.size() - 2);
    return ret;
  }

  arguments(int argc, const char** argv)
  {
    args::ArgumentParser parser("libremidi example");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

    args::ValueFlag<std::string> opt_api(parser, "api", "API to use (" + api_list() + ")", {'a'});
    args::ValueFlag<int> opt_port(parser, "input", "Input port to open", {'i'});
    args::ValueFlag<int> opt_out_port(parser, "output", "Output port to open", {'o'});
    args::ValueFlag<int> opt_count(parser, "count", "Number of bytes", {'n'});
    args::Flag opt_virt(
        parser, "virtual", "Open a virtual port instead of an existing one", {'v'});

    args::CompletionFlag completion(parser, {"complete"});

    try
    {
      parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
      std::cout << parser;
      std::exit(1);
    }
    catch (args::ParseError e)
    {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      std::exit(1);
    }
    catch (args::ValidationError e)
    {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      std::exit(1);
    }

    if (opt_api)
      api = libremidi::get_compiled_api_by_name(opt_api.Get());
    if (opt_port)
      input_port = opt_port.Get();
    if (opt_out_port)
      output_port = opt_out_port.Get();
    if (opt_count)
      count = opt_count.Get();
    if (opt_virt)
      virtual_port = true;
  }

  template <typename T>
  inline bool open_port(T& midi)
  {
    if (this->virtual_port)
    {
      const auto err = midi.open_virtual_port();
      return err == stdx::error{};
    }

    const auto obs = libremidi::observer{
        {.track_hardware = 1, .track_virtual = 1},
        observer_configuration_for(midi.get_current_api())};
    auto [ports, index] = get_ports(obs, midi);
    if (ports.empty())
    {
      std::cout << "No ports available!" << std::endl;
      return false;
    }

    if (index >= 0 && index < std::ssize(ports))
    {
      std::cout << "Opening " << ports[index].display_name << std::endl;
      const auto err = midi.open_port(ports[index]);
      return err == stdx::error{};
    }
    else
    {
      std::cout << "Cannot open port " << index << std::endl;
      return false;
    }
  }

private:
  auto get_ports(const libremidi::observer& obs, const libremidi::midi_in&)
  {
    return std::make_pair(obs.get_input_ports(), this->input_port);
  }
  auto get_ports(const libremidi::observer& obs, const libremidi::midi_out&)
  {
    return std::make_pair(obs.get_output_ports(), this->output_port);
  }
};
}

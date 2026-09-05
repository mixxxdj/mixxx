#pragma once
#include <libremidi/backends/net/config.hpp>
#include <libremidi/backends/net/midi_in.hpp>
#include <libremidi/backends/net/midi_out.hpp>
#include <libremidi/backends/net/observer.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI::net_ump
{
struct backend
{
  using midi_in = libremidi::net_ump::midi_in;
  using midi_out = libremidi::net_ump::midi_out;
  using midi_observer = libremidi::net_ump::observer;
  using midi_in_configuration = libremidi::net_ump::dgram_input_configuration;
  using midi_out_configuration = libremidi::net_ump::dgram_output_configuration;
  using midi_observer_configuration = libremidi::net_ump::net_observer_configuration;
  static const constexpr auto API = libremidi::API::NETWORK_UMP;
  static const constexpr std::string_view name = "network (UMP)";
  static const constexpr std::string_view display_name = "Network (UMP)";

  static inline bool available() noexcept { return true; }
};
}

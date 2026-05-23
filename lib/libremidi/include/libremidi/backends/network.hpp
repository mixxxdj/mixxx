#pragma once
#include <libremidi/backends/dummy.hpp>
#include <libremidi/backends/net/config.hpp>
#include <libremidi/backends/net/midi_in.hpp>
#include <libremidi/backends/net/midi_out.hpp>
#include <libremidi/backends/net/observer.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI::net
{
struct backend
{
  using midi_in = libremidi::net::midi_in;
  using midi_out = libremidi::net::midi_out;
  using midi_observer = libremidi::net::observer;
  using midi_in_configuration = libremidi::net::dgram_input_configuration;
  using midi_out_configuration = libremidi::net::dgram_output_configuration;
  using midi_observer_configuration = libremidi::net::net_observer_configuration;
  static const constexpr auto API = libremidi::API::NETWORK;
  static const constexpr std::string_view name = "network";
  static const constexpr std::string_view display_name = "Network";

  static inline bool available() noexcept { return true; }
};
}

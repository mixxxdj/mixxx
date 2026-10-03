#pragma once
#include <libremidi/backends/rawio/config.hpp>
#include <libremidi/backends/rawio/midi_in.hpp>
#include <libremidi/backends/rawio/midi_out.hpp>
#include <libremidi/backends/rawio/observer.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI::rawio
{
struct backend
{
  using midi_in = rawio::midi_in;
  using midi_out = rawio::midi_out;
  using midi_observer = rawio::observer;
  using midi_in_configuration = rawio_input_configuration;
  using midi_out_configuration = rawio_output_configuration;
  using midi_observer_configuration = rawio_observer_configuration;
  static const constexpr auto API = libremidi::API::RAW_IO;
  static const constexpr std::string_view name = "raw_io";
  static const constexpr std::string_view display_name = "Raw I/O";

  static inline bool available() noexcept { return true; }
};
}

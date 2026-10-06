#pragma once
#include <libremidi/backends/rawio/config.hpp>
#include <libremidi/backends/rawio/midi_in_ump.hpp>
#include <libremidi/backends/rawio/midi_out_ump.hpp>
#include <libremidi/backends/rawio/observer.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI::rawio_ump
{
struct backend
{
  using midi_in = rawio_ump::midi_in;
  using midi_out = rawio_ump::midi_out;
  using midi_observer = rawio_ump::observer;
  using midi_in_configuration = rawio_ump_input_configuration;
  using midi_out_configuration = rawio_ump_output_configuration;
  using midi_observer_configuration = rawio_ump_observer_configuration;
  static const constexpr auto API = libremidi::API::RAW_IO_UMP;
  static const constexpr std::string_view name = "raw_io_ump";
  static const constexpr std::string_view display_name = "Raw I/O (UMP)";

  static inline bool available() noexcept { return true; }
};
}

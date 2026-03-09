#pragma once
#include <libremidi/backends/dummy.hpp>
#include <libremidi/backends/linux/pipewire.hpp>
#include <libremidi/backends/pipewire_ump/config.hpp>
#include <libremidi/backends/pipewire_ump/midi_in.hpp>
#include <libremidi/backends/pipewire_ump/midi_out.hpp>
#include <libremidi/backends/pipewire_ump/observer.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI::pipewire_ump
{
struct backend
{
  using midi_in = libremidi::pipewire_ump::midi_in_pipewire;
  using midi_out = libremidi::pipewire_ump::midi_out_pipewire;
  using midi_observer = libremidi::pipewire_ump::observer_pipewire;
  using midi_in_configuration = libremidi::pipewire_ump::input_configuration;
  using midi_out_configuration = libremidi::pipewire_ump::output_configuration;
  using midi_observer_configuration = libremidi::pipewire_ump::observer_configuration;
  static const constexpr auto API = libremidi::API::PIPEWIRE_UMP;
  static const constexpr std::string_view name = "pipewire_ump";
  static const constexpr std::string_view display_name = "PipeWire (UMP)";

  static inline bool available() noexcept
  {
    static const libpipewire& pw = libpipewire::instance();
    if (!pw.available)
      return false;
    const std::string_view version = pw.get_library_version();
    if (version.size() >= 3)
    {
      if (version[0] > '1' || version[2] >= '4')
        return true;
    }
    return false;
  }
};
}

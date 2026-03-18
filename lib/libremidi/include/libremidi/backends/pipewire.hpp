#pragma once
#include <libremidi/backends/dummy.hpp>
#include <libremidi/backends/linux/pipewire.hpp>
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/backends/pipewire/helpers.hpp>
#include <libremidi/backends/pipewire/midi_in.hpp>
#include <libremidi/backends/pipewire/midi_out.hpp>
#include <libremidi/backends/pipewire/observer.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI::pipewire
{
struct backend
{
  using midi_in = midi_in_pipewire;
  using midi_out = midi_out_pipewire;
  using midi_observer = observer_pipewire;
  using midi_in_configuration = pipewire_input_configuration;
  using midi_out_configuration = pipewire_output_configuration;
  using midi_observer_configuration = pipewire_observer_configuration;
  static const constexpr auto API = libremidi::API::PIPEWIRE;
  static const constexpr std::string_view name = "pipewire";
  static const constexpr std::string_view display_name = "PipeWire";

  static inline bool available() noexcept
  {
    static const libpipewire& pw = libpipewire::instance();
    return pw.available;
  }
};
}

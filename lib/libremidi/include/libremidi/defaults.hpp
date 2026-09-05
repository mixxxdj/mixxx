#pragma once
#include <libremidi/api.hpp>
#include <libremidi/config.hpp>
#include <libremidi/configurations.hpp>
#include <libremidi/input_configuration.hpp>
#include <libremidi/observer_configuration.hpp>
#include <libremidi/output_configuration.hpp>

#include <optional>

NAMESPACE_LIBREMIDI
{
class midi_in;
class midi_out;
class observer;

//! Get the default midi in configuration object for a given API
LIBREMIDI_EXPORT
input_api_configuration midi_in_configuration_for(libremidi::API);

//! Get the default midi out configuration object for a given API
LIBREMIDI_EXPORT
output_api_configuration midi_out_configuration_for(libremidi::API);

//! Get the default observer configuration object for a given API
LIBREMIDI_EXPORT
observer_api_configuration observer_configuration_for(libremidi::API);

//! Get a matching midi in configuration object for a given observer instance.
LIBREMIDI_EXPORT
input_api_configuration midi_in_configuration_for(const libremidi::observer&);
//! Get a matching midi out configuration object for a given observer instance.
LIBREMIDI_EXPORT
output_api_configuration midi_out_configuration_for(const libremidi::observer&);

//! Get the default port for midi input (if any) in a given API.
//! Note that this will only return a hardware / physical port.
LIBREMIDI_EXPORT
std::optional<input_port> in_default_port(libremidi::API api) noexcept;

//! Get the default port for midi output (if any) in a given API.
//! Note that this will only return a hardware / physical port.
LIBREMIDI_EXPORT
std::optional<output_port> out_default_port(libremidi::API api) noexcept;

namespace midi1
{
//! Get a default configuration for creating a MIDI 1 input
LIBREMIDI_EXPORT
input_api_configuration in_default_configuration();

//! Get a default configuration for creating a MIDI 1 output
LIBREMIDI_EXPORT
output_api_configuration out_default_configuration();

//! Get a default configuration for creating a MIDI 1 observer
LIBREMIDI_EXPORT
observer_api_configuration observer_default_configuration();

//! Get a default MIDI 1 input port for the default API of the system
inline std::optional<input_port> in_default_port() noexcept
{
  return libremidi::in_default_port(default_api());
}

//! Get a default MIDI 1 output port for the default API of the system
inline std::optional<output_port> out_default_port() noexcept
{
  return libremidi::out_default_port(default_api());
}
}

namespace midi2
{
//! Get a default configuration for creating a MIDI 2 input
LIBREMIDI_EXPORT
input_api_configuration in_default_configuration();

//! Get a default configuration for creating a MIDI 2 output
LIBREMIDI_EXPORT
output_api_configuration out_default_configuration();

//! Get a default configuration for creating a MIDI 2 observer
LIBREMIDI_EXPORT
observer_api_configuration observer_default_configuration();

//! Get a default MIDI 2 input port for the default API of the system
inline std::optional<input_port> in_default_port() noexcept
{
  return libremidi::in_default_port(default_api());
}
//! Get a default MIDI 2 output port for the default API of the system
inline std::optional<output_port> out_default_port() noexcept
{
  return libremidi::out_default_port(default_api());
}
}
}

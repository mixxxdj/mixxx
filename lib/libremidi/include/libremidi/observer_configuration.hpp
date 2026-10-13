#pragma once
#include <libremidi/port_information.hpp>

NAMESPACE_LIBREMIDI
{
using input_port_callback = std::function<void(const input_port&)>;
using output_port_callback = std::function<void(const output_port&)>;
struct observer_configuration
{
  midi_error_callback on_error{};
  midi_warning_callback on_warning{};

  input_port_callback input_added{};
  input_port_callback input_removed{};
  output_port_callback output_added{};
  output_port_callback output_removed{};

  // Observe hardware ports
  uint32_t track_hardware : 1 = true;

  // Observe software (virtual) ports if the API provides it
  uint32_t track_virtual : 1 = false;

  // Observe network ports if the API provides it
  uint32_t track_network : 1 = false;

  // Observe any port - some systems have other weird port types than hw / sw, this covers them
  uint32_t track_any : 1 = false;

  // Notify of the existing ports in the observer constructor
  uint32_t notify_in_constructor : 1 = true;

  bool has_callbacks() const noexcept
  {
    return input_added || input_removed || output_added || output_removed;
  }
};
}

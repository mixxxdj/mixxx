#pragma once
#include <libremidi/backends/coremidi/config.hpp>

NAMESPACE_LIBREMIDI::coremidi_ump
{
struct input_configuration
{
  std::string client_name = "libremidi client";
  std::optional<MIDIClientRef> context{};
};

struct output_configuration
{
  std::string client_name = "libremidi client";
  std::optional<MIDIClientRef> context{};
};

struct observer_configuration
{
  std::string client_name = "libremidi client";
  std::function<void(MIDIClientRef)> on_create_context{};
};
}

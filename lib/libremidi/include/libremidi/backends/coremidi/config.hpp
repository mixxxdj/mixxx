#pragma once
#include <libremidi/config.hpp>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#if defined(__APPLE__)
  #if __LP64__
typedef unsigned int UInt32;
  #else
typedef unsigned long UInt32;
  #endif
typedef UInt32 MIDIObjectRef;
typedef MIDIObjectRef MIDIClientRef;
#else
using MIDIClientRef = uint32_t;
using MIDIObjectRef = uint32_t;
#endif
NAMESPACE_LIBREMIDI
{

struct coremidi_input_configuration
{
  std::string client_name = "libremidi client";
  std::optional<MIDIClientRef> context{};
};

struct coremidi_output_configuration
{
  std::string client_name = "libremidi client";
  std::optional<MIDIClientRef> context{};
};

struct coremidi_observer_configuration
{
  std::string client_name = "libremidi client";
  std::function<void(MIDIClientRef)> on_create_context{};
};

}

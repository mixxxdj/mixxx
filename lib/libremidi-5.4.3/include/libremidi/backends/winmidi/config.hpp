#pragma once
#include <libremidi/config.hpp>

#include <string>

#if !defined(LIBREMIDI_MODULE_BUILD) || !defined(_WIN32)
namespace winrt::Microsoft::Windows::Devices::Midi2
{
struct MidiSession;
struct MidiEndpointDeviceInformation;
struct MidiEndpointConnection;
}
#endif

// TODO allow to share midi session and endpoints
NAMESPACE_LIBREMIDI::winmidi
{

struct input_configuration
{
  std::string client_name = "libremidi input";
  winrt::Microsoft::Windows::Devices::Midi2::MidiSession* context{};
};

struct output_configuration
{
  std::string client_name = "libremidi output";
  winrt::Microsoft::Windows::Devices::Midi2::MidiSession* context{};
};

struct observer_configuration
{
  std::string client_name = "libremidi observer";
};

}

#pragma once
#include <libremidi/backends/android/config.hpp>
#include <libremidi/backends/android/midi_in.hpp>
#include <libremidi/backends/android/midi_out.hpp>
#include <libremidi/backends/android/observer.hpp>

NAMESPACE_LIBREMIDI
{
namespace android
{
struct backend
{
  using midi_in = android::midi_in;
  using midi_out = android::midi_out;
  using midi_observer = android::observer;
  using midi_in_configuration = android::input_configuration;
  using midi_out_configuration = android::output_configuration;
  using midi_observer_configuration = android::observer_configuration;
  static const constexpr auto API = libremidi::API::ANDROID_AMIDI;
  static const constexpr std::string_view name = "android";
  static const constexpr std::string_view display_name = "Android MIDI API";

  static bool available() noexcept
  {
#if defined(__ANDROID__)
    return true;
#else
    return false;
#endif
  }
};
}
}

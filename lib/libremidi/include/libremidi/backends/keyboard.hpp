#pragma once
#include <libremidi/backends/dummy.hpp>
#include <libremidi/backends/keyboard/midi_in.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI
{
struct kbd_backend
{
  using midi_in = midi_in_kbd;
  using midi_in_configuration = kbd_input_configuration;
  using midi_out = midi_out_dummy;
  using midi_observer = observer_dummy;
  using midi_out_configuration = dummy_configuration;
  using midi_observer_configuration = dummy_configuration;
  static const constexpr auto API = libremidi::API::KEYBOARD;
  static const constexpr std::string_view name = "keyboard";
  static const constexpr std::string_view display_name = "Computer keyboard";

  static inline bool available() noexcept { return true; }
};
}

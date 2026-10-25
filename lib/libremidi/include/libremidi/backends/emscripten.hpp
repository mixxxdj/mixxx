#pragma once

#if defined(__EMSCRIPTEN__)

  #include <libremidi/backends/emscripten/config.hpp>
  #include <libremidi/backends/emscripten/midi_in.hpp>
  #include <libremidi/backends/emscripten/midi_out.hpp>
  #include <libremidi/backends/emscripten/observer.hpp>

  #include <string_view>

NAMESPACE_LIBREMIDI
{

struct emscripten_backend
{
  using midi_in = midi_in_emscripten;
  using midi_out = midi_out_emscripten;
  using midi_observer = observer_emscripten;
  using midi_in_configuration = emscripten_input_configuration;
  using midi_out_configuration = emscripten_output_configuration;
  using midi_observer_configuration = emscripten_observer_configuration;
  static const constexpr auto API = libremidi::API::WEBMIDI;
  static const constexpr std::string_view name = "webmidi";
  static const constexpr std::string_view display_name = "WebMIDI";

  static constexpr inline bool available() noexcept { return true; }
};

}

#endif

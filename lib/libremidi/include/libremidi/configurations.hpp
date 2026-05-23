#pragma once

#include <libremidi/api.hpp>
#include <libremidi/backends/alsa_raw/config.hpp>
#include <libremidi/backends/alsa_raw_ump/config.hpp>
#include <libremidi/backends/alsa_seq/config.hpp>
#include <libremidi/backends/alsa_seq_ump/config.hpp>
#include <libremidi/backends/android/config.hpp>
#include <libremidi/backends/coremidi/config.hpp>
#include <libremidi/backends/coremidi_ump/config.hpp>
#include <libremidi/backends/emscripten/config.hpp>
#include <libremidi/backends/jack/config.hpp>
#include <libremidi/backends/kdmapi/config.hpp>
#include <libremidi/backends/jack_ump/config.hpp>
#include <libremidi/backends/keyboard/config.hpp>
#include <libremidi/backends/net/config.hpp>
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/backends/pipewire_ump/config.hpp>
#include <libremidi/backends/winmidi/config.hpp>
#include <libremidi/backends/winmm/config.hpp>
#include <libremidi/backends/winuwp/config.hpp>
#include <libremidi/config.hpp>

#include <variant>
NAMESPACE_LIBREMIDI
{
struct unspecified_configuration
{
};
struct dummy_configuration
{
};

using input_api_configuration = libremidi_variant_alias::variant<
    unspecified_configuration, dummy_configuration, alsa_raw_input_configuration,
    alsa_raw_ump::input_configuration, alsa_seq::input_configuration,
    alsa_seq_ump::input_configuration, coremidi_input_configuration,
    coremidi_ump::input_configuration, emscripten_input_configuration, jack_input_configuration,
    kbd_input_configuration, kdmapi::input_configuration,
    libremidi::net::dgram_input_configuration, libremidi::net_ump::dgram_input_configuration,
    pipewire_input_configuration, winmidi::input_configuration, winmm_input_configuration,
    winuwp_input_configuration, jack_ump::input_configuration, pipewire_ump::input_configuration,
    android::input_configuration, libremidi::API>;

using output_api_configuration = libremidi_variant_alias::variant<
    unspecified_configuration, dummy_configuration, alsa_raw_output_configuration,
    alsa_raw_ump::output_configuration, alsa_seq::output_configuration,
    alsa_seq_ump::output_configuration, coremidi_output_configuration,
    coremidi_ump::output_configuration, emscripten_output_configuration, jack_output_configuration,
    kdmapi::output_configuration, libremidi::net::dgram_output_configuration,
    libremidi::net_ump::dgram_output_configuration, pipewire_output_configuration,
    winmidi::output_configuration, winmm_output_configuration, winuwp_output_configuration,
    jack_ump::output_configuration, pipewire_ump::output_configuration,
    android::output_configuration, libremidi::API>;

using observer_api_configuration = libremidi_variant_alias::variant<
    unspecified_configuration, dummy_configuration, alsa_raw_observer_configuration,
    alsa_raw_ump::observer_configuration, alsa_seq::observer_configuration,
    alsa_seq_ump::observer_configuration, coremidi_observer_configuration,
    coremidi_ump::observer_configuration, emscripten_observer_configuration,
    jack_observer_configuration, kdmapi::observer_configuration,
    libremidi::net::dgram_observer_configuration, libremidi::net_ump::dgram_observer_configuration,
    pipewire_observer_configuration, winmidi::observer_configuration, winmm_observer_configuration,
    winuwp_observer_configuration, jack_ump::observer_configuration,
    pipewire_ump::observer_configuration, android::observer_configuration, libremidi::API>;

LIBREMIDI_EXPORT
libremidi::API midi_api(const input_api_configuration& conf);
LIBREMIDI_EXPORT
libremidi::API midi_api(const output_api_configuration& conf);
LIBREMIDI_EXPORT
libremidi::API midi_api(const observer_api_configuration& conf);

void set_client_name(auto&& conf, std::string name)
{
  std::visit([&](auto& impl) {
    if constexpr (requires { impl.client_name; })
      impl.client_name = std::move(name);
  }, conf);
}

std::optional<std::string> client_name(const auto& conf) noexcept
{
  return std::visit([&](auto& impl) -> std::optional<std::string> {
    if constexpr (requires { impl.client_name; })
      return impl.client_name;
    return std::nullopt;
  }, conf);
}
}
